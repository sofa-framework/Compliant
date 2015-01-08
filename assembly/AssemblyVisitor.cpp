#include "AssemblyVisitor.h"


#include <SofaEigen2Solver/EigenVectorWrapper.h>
#include <SofaBaseLinearSolver/SingleMatrixAccessor.h>
#include <SofaBaseLinearSolver/DefaultMultiMatrixAccessor.h>

#include "./utils/scoped.h"
#include "./utils/cast.h"
#include "./utils/sparse.h"

#include "../constraint/ConstraintValue.h"
#include "../constraint/Stabilization.h"

using std::cerr;
using std::endl;


namespace sofa {
namespace simulation {

using namespace component::linearsolver;
using namespace core::behavior;


AssemblyVisitor::AssemblyVisitor(const core::MechanicalParams* mparams)
	: base( mparams ),
      mparams( mparams ),
	  start_node(0),
	  _processed(0)
{
    mparamsWithoutStiffness = *mparams;
    mparamsWithoutStiffness.setKFactor(0);
}


AssemblyVisitor::~AssemblyVisitor()
{
	if( _processed ) delete _processed;
}


AssemblyVisitor::chunk::chunk()
	: offset(0),
	  size(0),
	  mechanical(false),
	  vertex(-1),
	  dofs(0) {

}

// pretty prints a mapping
static inline std::string mapping_name(simulation::Node* node) {
	return node->mechanicalMapping->getName() + " (class: " + node->mechanicalMapping->getClassName() + ") ";
}

// mapping informations as a map (parent dofs -> J matrix )
AssemblyVisitor::chunk::map_type AssemblyVisitor::mapping(simulation::Node* node) {
	chunk::map_type res;

	if( !node->mechanicalMapping ) return res;

	using helper::vector;

	assert( node->mechanicalMapping->getTo().size() == 1 &&
	        "only n -> 1 mappings are handled");

    ForceMaskActivate(node->mechanicalMapping->getMechTo());
    ForceMaskActivate(node->mechanicalMapping->getMechFrom());

    const vector<sofa::defaulttype::BaseMatrix*>* js = node->mechanicalMapping->getJs();

    ForceMaskDeactivate(node->mechanicalMapping->getMechTo());


    vector<core::BaseState*> from = node->mechanicalMapping->getFrom();

    for( unsigned i = 0, n = from.size(); i < n; ++i ) {

		// parent dofs
		dofs_type* p = safe_cast<dofs_type>(from[i]);

		// skip non-mechanical dofs
		if(!p) continue;

        if( !notempty((*js)[i]) )
        {
            std::cerr<<"ERROR: AssemblyVisitor: empty mapping block for " + mapping_name(node) + " (is mapping matrix assembled?)"<<std::endl;
            continue;
        }

		// mapping wrt p
        chunk::mapped& c = res[p];

        // getting BaseMatrix pointer
        c.J = (*js)[i];
	}

	return res;
}






// projection matrix
AssemblyVisitor::mat AssemblyVisitor::proj(simulation::Node* node) {
	assert( node->mechanicalState );

	unsigned size = node->mechanicalState->getMatrixSize();

	// identity matrix TODO alloc ?
    tmp_p.compressedMatrix = shift_right<mat>(0, size, size);

	for(unsigned i=0; i<node->projectiveConstraintSet.size(); i++){
		node->projectiveConstraintSet[i]->projectMatrix(&tmp_p, 0);
        isPIdentity = false;
	}

	return tmp_p.compressedMatrix;
}




const defaulttype::BaseMatrix* compliance_impl( const core::MechanicalParams* mparams, BaseForceField* ffield )
{
    const defaulttype::BaseMatrix* c = ffield->getComplianceMatrix(mparams);


    if( notempty(c) )
    {
        return c;
    }
    else
    {
        std::cerr<<"AssemblyVisitor::compliance: "<<ffield->getName()<<" getComplianceMatrix not implemented"<< std::endl;
        // TODO inverting stiffness matrix
    }

    return NULL;
}


// compliance matrix
const defaulttype::BaseMatrix* AssemblyVisitor::compliance(simulation::Node* node)
{

    for(unsigned i = 0; i < node->forceField.size(); ++i )
    {
		BaseForceField* ffield = node->forceField[i];

		if( !ffield->isCompliance.getValue() ) continue;

        return compliance_impl( mparams, ffield );
	}

    for(unsigned i = 0; i < node->interactionForceField.size(); ++i )
    {
        BaseInteractionForceField* ffield = node->interactionForceField[i];

        if( !ffield->isCompliance.getValue() ) continue;

        if( ffield->getMechModel1() != ffield->getMechModel2() )
        {
            std::cerr<<SOFA_CLASS_METHOD<<"WARNING: interactionForceField "<<ffield->getName()<<" cannot be simulated as a compliance."<<std::endl;
        }
        else
        {
            return compliance_impl( mparams, ffield );
        }
    }

    return NULL;
}


// geometric stiffness matrix
const defaulttype::BaseMatrix* AssemblyVisitor::geometricStiffness(simulation::Node* node)
{
//    std::cerr<<"AssemblyVisitor::geometricStiffness "<<node->getName()<<" "<<node->mechanicalMapping->getName()<<std::endl;

    core::BaseMapping* mapping = node->mechanicalMapping;
    if( mapping )
    {
        const sofa::defaulttype::BaseMatrix* k = mapping->getK();
        if( k ) return k;
    }

    return NULL;
}



// interaction forcefied in a node w/o a mechanical state
void AssemblyVisitor::interactionForceField(simulation::Node* node)
{
    for(unsigned i = 0; i < node->interactionForceField.size(); ++i )
    {
        BaseInteractionForceField* ffield = node->interactionForceField[i];

        if( ffield->getMechModel1() != ffield->getMechModel2() )
        {
            typedef EigenBaseSparseMatrix<SReal> BigSqmat;
            unsigned bigsize = ffield->getMechModel1()->getMatrixSize() + ffield->getMechModel2()->getMatrixSize();
            BigSqmat bigSqmat( bigsize, bigsize );
            DefaultMultiMatrixAccessor accessor;
            accessor.setGlobalMatrix( &bigSqmat );
            accessor.addMechanicalState( ffield->getMechModel1() );
            accessor.addMechanicalState( ffield->getMechModel2() );

            // an interactionFF is always a stiffness
            ffield->addMBKToMatrix( mparams, &accessor );
            bigSqmat.compress();

            if( !zero(bigSqmat.compressedMatrix) )
                interactionForceFieldList.push_back( InteractionForceField(bigSqmat.compressedMatrix,ffield) );


//            std::cerr<<"AssemblyVisitor::interactionForceField "<<ffield->getMechModel1()->getName()<<" "<<ffield->getMechModel2()->getName()<<" "<<bigSqmat<<std::endl;
        }
    }
}



// ode matrix
AssemblyVisitor::mat AssemblyVisitor::odeMatrix(simulation::Node* node)
{
    unsigned size = node->mechanicalState->getMatrixSize();

    typedef EigenBaseSparseMatrix<SReal> Sqmat;
    Sqmat sqmat( size, size );

    for(unsigned i = 0; i < node->interactionForceField.size(); ++i )
    {
        BaseInteractionForceField* ffield = node->interactionForceField[i];

        if( ffield->getMechModel1() != ffield->getMechModel2() )
        {
//            std::cerr<<SOFA_CLASS_METHOD<<"WARNING: interactionForceField "<<ffield->getName()<<" will be treated as explicit, external forces (interactionForceFields are not handled by Compliant assembly, the same scene should be modelised with MultiMappings)"<<std::endl;

            typedef EigenBaseSparseMatrix<SReal> BigSqmat;
            unsigned bigsize = ffield->getMechModel1()->getMatrixSize() + ffield->getMechModel2()->getMatrixSize();
            BigSqmat bigSqmat( bigsize, bigsize );
            DefaultMultiMatrixAccessor accessor;
            accessor.setGlobalMatrix( &bigSqmat );
            accessor.addMechanicalState( ffield->getMechModel1() );
            accessor.addMechanicalState( ffield->getMechModel2() );

            // an interactionFF is always a stiffness
            ffield->addMBKToMatrix( mparams, &accessor );
            bigSqmat.compress();

            if( !zero(bigSqmat.compressedMatrix) )
                interactionForceFieldList.push_back( InteractionForceField(bigSqmat.compressedMatrix,ffield) );


//            std::cerr<<"AssemblyVisitor::odeMatrix "<<ffield->getName()<<" "<<bigSqmat<<std::endl;
        }
        else
        {
            // interactionForceFields that work on a unique set of dofs are OK
            SingleMatrixAccessor accessor( &sqmat );

            // when it is a compliant, you need to add M if mass, B but not K
            ffield->addMBKToMatrix( ffield->isCompliance.getValue() ? &mparamsWithoutStiffness : mparams, &accessor );
        }
    }

    // note that mass are included in forcefield
    for(unsigned i = 0; i < node->forceField.size(); ++i )
    {
        BaseForceField* ffield = node->forceField[i];

        SingleMatrixAccessor accessor( &sqmat );

        // when it is a compliant, you need to add M if mass, B but not K
        ffield->addMBKToMatrix( ffield->isCompliance.getValue() ? &mparamsWithoutStiffness : mparams, &accessor );
    }

    sqmat.compress();
    return sqmat.compressedMatrix.selfadjointView<Eigen::Upper>();
}


void AssemblyVisitor::top_down(simulation::Visitor* vis) const {
	assert( !prefix.empty() );

	for(unsigned i = 0, n = prefix.size(); i < n; ++i) {
		simulation::Node* node = down_cast<simulation::Node>(graph[ prefix[i] ].dofs->getContext());
		vis->processNodeTopDown( node );
	}

}

void AssemblyVisitor::bottom_up(simulation::Visitor* vis) const {
	assert( !prefix.empty() );

	for(unsigned i = 0, n = prefix.size(); i < n; ++i) {
		simulation::Node* node = down_cast<simulation::Node>(graph[ prefix[ n - 1 - i] ].dofs->getContext());
		vis->processNodeBottomUp( node );
	}
	
}





// this is called on the *top-down* traversal, once for each node. we
// simply fetch infos for each dof.
void AssemblyVisitor::fill_prefix(simulation::Node* node) {
	assert( node->mechanicalState );
    assert( chunks.find( node->mechanicalState ) == chunks.end() );

	// fill chunk for current dof
	chunk& c = chunks[ node->mechanicalState ];

	c.size = node->mechanicalState->getMatrixSize();
	c.dofs = node->mechanicalState;

	vertex v; v.dofs = c.dofs; v.data = &c;

	c.H = odeMatrix( node );
//    cerr << "AssemblyVisitor::fill_prefix, c.H = " << endl << dmat(c.H) << endl;

	if( !zero(c.H) ) {
		c.mechanical = true;
	}


    // if the visitor is excecuted from a mapped node, do not look at its mapping
    if( node != start_node ) c.map = mapping( node );
	
	c.vertex = boost::add_vertex(graph);
	graph[c.vertex] = v;

	// independent dofs
	if( c.map.empty() ) {
		c.P = proj( node );
	} else {
		// mapped dofs

        // compliance
		c.C = compliance( node );
        if( notempty(c.C) ) {
			c.mechanical = true;
		}

        // geometric stiffness
        c.Ktilde = geometricStiffness( node );
	}
}



// bottom-up: build dependency graph
void AssemblyVisitor::fill_postfix(simulation::Node* node) {
	assert( node->mechanicalState );
	assert( chunks.find( node->mechanicalState ) != chunks.end() );

	// fill chunk for current dof
	chunk& c = chunks[ node->mechanicalState ];

	for(chunk::map_type::const_iterator it = c.map.begin(), end = c.map.end();
	    it != end; ++it) {

        if( chunks.find(it->first) == chunks.end() ) continue; // this mechanical object is out of scope (ie not in the sub-graph controled by this solver)
        chunk& p = chunks[it->first];

		edge e;
		e.data = &it->second;

		// the edge is child -> parent
		graph_type::edge_descriptor ed = boost::add_edge(c.vertex, p.vertex, graph).first;
		graph[ed] = e;
	}

}




void AssemblyVisitor::chunk::debug() const {
	using namespace std;

	cout << "chunk: " << dofs->getName() << endl
	     << "offset:" << offset << endl
	     << "size: " << size << endl
	     << "H:" << endl << H << endl
	     << "P:" << endl << P << endl
	     << "C:" << endl << C << endl
         << "Ktilde:" << endl << Ktilde << endl
	     << "map: " << endl
		;

	for(map_type::const_iterator mi = map.begin(), me = map.end();
	    mi != me; ++mi) {
		cout << "from: " << mi->first->getName() << endl
		     << "J: " << endl
             << mi->second.J << endl
			;
	}
}

void AssemblyVisitor::debug() const {

	for(chunks_type::const_iterator i = chunks.begin(), e = chunks.end(); i != e; ++i ) {
		i->second.debug();
	}

}



// this is used to propagate mechanical flag upwards mappings (prefix
// in the graph order)
struct AssemblyVisitor::propagation_helper {

	const core::MechanicalParams* mparams;
	graph_type& g;

	propagation_helper(const core::MechanicalParams* mparams, graph_type& g) : mparams(mparams), g(g) {}

    void operator()( unsigned v ) const {

		chunk* c = g[v].data;

        // if the current child is a mechanical dof
        if( c->mechanical ) {

            // have a look to all its parents
			for(graph_type::out_edge_range e = boost::out_edges(v, g); e.first != e.second; ++e.first) {

				chunk* p = g[ boost::target(*e.first, g) ].data;
                p->mechanical = true; // a parent of a mechanical child is necessarily mechanical
            }

		}
	}

};




struct AssemblyVisitor::prefix_helper {
	prefix_type& res;

	prefix_helper(prefix_type& res) : res(res) {
		res.clear();
	}

	template<class G>
	void operator()(unsigned v, const G& ) const {
		res.push_back( v );
	}

};



AssemblyVisitor::process_type* AssemblyVisitor::process() const {
	// scoped::timer step("mapping processing");

    process_type* res = new process_type();

    unsigned& size_m = res->size_m;
    unsigned& size_c = res->size_c;

	// independent dofs offsets (used for shifting parent)
    offset_type& offsets = res->offset.master;

	unsigned off_m = 0;
	unsigned off_c = 0;

	for(unsigned i = 0, n = prefix.size(); i < n; ++i) {

		const chunk* c = graph[ prefix[i] ].data;

		// independent
		if( c->master() ) {
			offsets[ graph[ prefix[i] ].dofs ] = off_m;
			off_m += c->size;
        } else if( notempty(c->C) ) {
			off_c += c->size;
		}

	}

	// update total sizes
	size_m = off_m;
	size_c = off_c;

	// prefix mapping concatenation and stuff
    std::for_each(prefix.begin(), prefix.end(), process_helper(*res, graph) ); 	// TODO merge with offsets computation ?


    // special treatment for interaction forcefields
    fullmapping_type& full = res->fullmapping;
    for( InteractionForceFieldList::iterator it=interactionForceFieldList.begin(),itend=interactionForceFieldList.end();it!=itend;++it)
    {
        mat& Jp0 = full[ it->ff->getMechModel1() ];
        mat& Jp1 = full[ it->ff->getMechModel2() ];

        if( empty(Jp0) ) {
            Jp0 = shift_right<mat>( find(offsets, it->ff->getMechModel1()), it->ff->getMechModel1()->getMatrixSize(), size_m);
        }
        if( empty(Jp1) ) {
            Jp1 = shift_right<mat>( find(offsets, it->ff->getMechModel2()), it->ff->getMechModel2()->getMatrixSize(), size_m);
        }

        it->J.resize( it->H.rows(), size_m );

        add( it->J, shift_left<mat>( 0, it->ff->getMechModel1()->getMatrixSize(), it->H.rows() ) * Jp0 );
        add( it->J, shift_left<mat>( it->ff->getMechModel1()->getMatrixSize(), it->ff->getMechModel2()->getMatrixSize(), it->H.rows() ) * Jp1 );
    }

	return res;
}




// this is meant to optimize L^T D L products
static inline AssemblyVisitor::mat ltdl(const AssemblyVisitor::mat& l,
                                        const AssemblyVisitor::mat& d)
{
//#ifdef USING_OMP_PRAGMAS
//    return component::linearsolver::mul_EigenSparseMatrix_MT( l.transpose(), component::linearsolver::mul_EigenSparseMatrix_MT( d, l ) );
//#else
    return l.transpose() * (d * l);
//#endif
}




// produce actual system assembly
AssemblyVisitor::system_type AssemblyVisitor::assemble() const {
	scoped::timer step("assembly");
	assert(!chunks.empty() && "need to send a visitor first");

	// assert( !_processed );

	// concatenate mappings and obtain sizes
    _processed = process();

	// result system
    system_type res(_processed->size_m, _processed->size_c);

	res.dt = mparams->dt();
    res.isPIdentity = isPIdentity;



    // Geometric Stiffness must be processed first, from mapped dofs to master dofs
    // warning, inverse order is important, to treat mapped dofs before master dofs
    // so mapped dofs can transfer their geometric stiffness to master dofs that will add it to the assembled matrix
    for( int i = (int)prefix.size()-1 ; i >=0 ; --i ) {

        const chunk& c = *graph[ prefix[i] ].data;
        assert( c.size );

        // only consider mechanical mapped dofs that have geometric stiffness
        if( !c.mechanical || c.master() || !c.Ktilde ) continue;

        // TODO remove copy for matrices that are already in the right type (EigenBaseSparseMatrix<SReal>)
        MySPtr<mat> Ktilde( convertSPtr<mat>( c.Ktilde ) );

        if( zero( *Ktilde ) ) continue;

        if( boost::out_degree(prefix[i],graph) == 1 ) // simple mapping
        {
//            std::cerr<<"simple mapping "<<c.dofs->getName()<<std::endl;
            // add the geometric stiffness to its only parent that will map it to the master level
            graph_type::out_edge_iterator parentIterator = boost::out_edges(prefix[i],graph).first;
            chunk* p = graph[ boost::target(*parentIterator, graph) ].data;
            add(p->H, mparams->kFactor() * *Ktilde ); // todo how to include rayleigh damping for geometric stiffness?

//            std::cerr<<"Assembly: "<<c.Ktilde<<std::endl;
        }
        else // multimapping
        {
//            std::cerr<<"multimapping "<<c.dofs->getName()<<std::endl;
            // directly add the geometric stiffness to the assembled level
            // by mapping with the specific jacobian from master to the (current-1) level



            // full mapping chunk for geometric stiffness
            const mat& geometricStiffnessJc = _processed->fullmappinggeometricstiffness[ graph[ prefix[i] ].dofs ];
//std::cerr<<geometricStiffnessJc<<std::endl;
//std::cerr<<c.Ktilde<<std::endl;

//            std::cerr<<res.H.rows()<<" "<<geometricStiffnessJc.rows()<<std::endl;

            res.H += ltdl(geometricStiffnessJc, mparams->kFactor() * *Ktilde);
        }

    }


    // Then add interaction forcefields
    for( InteractionForceFieldList::iterator it=interactionForceFieldList.begin(),itend=interactionForceFieldList.end();it!=itend;++it)
    {
        res.H += ltdl(it->J, it->H);
    }




	// master/compliant offsets
	unsigned off_m = 0;
	unsigned off_c = 0;

#if USE_TRIPLETS_RATHER_THAN_SHIFT_MATRIX
    typedef Eigen::Triplet<real> Triplet;
    std::vector<Triplet> H_triplets, C_triplets, P_triplets;
#elif USE_DENSEMATRIX_RATHER_THAN_SHIFT_MATRIX
    typedef Eigen::Matrix<real, Eigen::Dynamic, Eigen::Dynamic> DenseMat;
    DenseMat H(_processed->size_m,_processed->size_m), C(_processed->size_c, _processed->size_c), P(_processed->size_m,_processed->size_m);
#endif

	// assemble system
    for( unsigned i = 0, n = prefix.size() ; i < n ; ++i ) {

		// current chunk
        const chunk& c = *graph[ prefix[i] ].data;
        assert( c.size );

        if( !c.mechanical ) continue;

		// independent dofs: fill mass/stiffness
        if( c.master() ) {
            res.master.push_back( c.dofs );


            // scoped::timer step("independent dofs");
#if USE_TRIPLETS_RATHER_THAN_SHIFT_MATRIX
            if( !zero(c.H) ) add_shifted_right<Triplet,mat>( H_triplets, c.H, off_m );
            if( !zero(c.P) ) add_shifted_right<Triplet,mat>( P_triplets, c.P, off_m );
#elif USE_SPARSECOEFREF_RATHER_THAN_SHIFT_MATRIX
            if( !zero(c.H) ) add_shifted_right<mat>( res.H, c.H, off_m );
            if( !zero(c.P) ) add_shifted_right<mat>( res.P, c.P, off_m );
#elif USE_DENSEMATRIX_RATHER_THAN_SHIFT_MATRIX
            if( !zero(c.H) ) add_shifted_right<DenseMat,mat>( H, c.H, off_m );
            if( !zero(c.P) ) add_shifted_right<DenseMat,mat>( P, c.P, off_m );
#elif SHIFTING_MATRIX_WITHOUT_MULTIPLICATION
            if( !zero(c.H) ) res.H.middleRows(off_m, c.size) = res.H.middleRows(off_m, c.size) + shifted_matrix( c.H, off_m, _processed->size_m );
            if( !zero(c.P) ) res.P.middleRows(off_m, c.size) = shifted_matrix( c.P, off_m, _processed->size_m );
#else
            mat shift = shift_right<mat>(off_m, c.size, _processed->size_m);
            if( !zero(c.H) ) res.H.middleRows(off_m, c.size) = res.H.middleRows(off_m, c.size) + c.H * shift;
            if( !zero(c.P) ) res.P.middleRows(off_m, c.size) = c.P * shift;

#endif

            off_m += c.size;

		}

		// mapped dofs
		else {

            // full mapping chunk
            const mat& Jc = _processed->fullmapping[ graph[ prefix[i] ].dofs ];

			if( !zero(Jc) ) {
                assert( Jc.cols() == int(_processed->size_m) );

                // actual response matrix mapping
                if( !zero(c.H) ) {

#if USE_TRIPLETS_RATHER_THAN_SHIFT_MATRIX
                    add_shifted_right( H_triplets, ltdl(Jc, c.H), 0 );
#elif USE_DENSEMATRIX_RATHER_THAN_SHIFT_MATRIX
                    add_shifted_right( H, ltdl(Jc, c.H), 0 );
#else
                    res.H += ltdl(Jc, c.H);
#endif
                }
            }


			// compliant dofs: fill compliance/phi/lambda
			if( c.compliant() ) {
				res.compliant.push_back( c.dofs );
				// scoped::timer step("compliant dofs");
				assert( !zero(Jc) );


                // TODO remove copy for matrices that are already in the right type (EigenBaseSparseMatrix<SReal>)
                MySPtr<mat> C( convertSPtr<mat>( c.C ) );


                // fetch projector and constraint value if any
                AssembledSystem::constraint_type constraint;
                constraint.projector = c.dofs->getContext()->get<component::linearsolver::Constraint>( core::objectmodel::BaseContext::Local );
                constraint.value = c.dofs->getContext()->get<component::odesolver::BaseConstraintValue>( core::objectmodel::BaseContext::Local );

                // by default the manually given ConstraintValue is used
                // otherwise a fallback is used depending on the constraint type
                if( !constraint.value ) {

                    // a non bilateral constraint should be stabilizable, as a non compliant (hard) bilateral constraint
                    if( constraint.projector || zero(*C) /*|| fillWithZeros(*C)*/ ) constraint.value = new component::odesolver::Stabilization( c.dofs );
                    // by default, a compliant bilateral constraint is considered as elastic and is so not stabilized
                    else constraint.value = new component::odesolver::ConstraintValue( c.dofs );

                    c.dofs->getContext()->addObject( constraint.value );
                    constraint.value->init();
                }
                res.constraints.push_back( constraint );


				// mapping
				res.J.middleRows(off_c, c.size) = Jc;

                // compliance

                if( !zero( *C ) )
                {
                    SReal factor = 1.0 /
                        ( res.dt * res.dt * mparams->implicitVelocity() * mparams->implicitPosition() );
					
#if USE_TRIPLETS_RATHER_THAN_SHIFT_MATRIX
                        add_shifted_right( C_triplets, *C, off_c, factor );
#elif USE_SPARSECOEFREF_RATHER_THAN_SHIFT_MATRIX
                        add_shifted_right( res.C, *C, off_c, factor );
#elif USE_DENSEMATRIX_RATHER_THAN_SHIFT_MATRIX
                        add_shifted_right( *C, C, off_c, factor );
#elif SHIFTING_MATRIX_WITHOUT_MULTIPLICATION
                        res.C.middleRows(off_c, c.size) = shifted_matrix( *C, off_c, _processed->size_c, factor );
#else
                        res.C.middleRows(off_c, c.size) = *C * shift_right<mat>(off_c, c.size, _processed->size_c, factor);
#endif
                }
				off_c += c.size;
			}
		}
	}
	
#if USE_TRIPLETS_RATHER_THAN_SHIFT_MATRIX
    res.H.setFromTriplets( H_triplets.begin(), H_triplets.end() );
    res.P.setFromTriplets( P_triplets.begin(), P_triplets.end() );
    res.C.setFromTriplets( C_triplets.begin(), C_triplets.end() );
#elif USE_DENSEMATRIX_RATHER_THAN_SHIFT_MATRIX
    convertDenseToSparse( res.H, H );
    convertDenseToSparse( res.C, C );
    convertDenseToSparse( res.P, P );
#endif

    assert( off_m == _processed->size_m );
    assert( off_c == _processed->size_c );

    /// \warning project the ODE matrices
	// max: no we don't
    // res.H = res.P.transpose() * res.H * res.P;

	return res;
}

// TODO redo
bool AssemblyVisitor::chunk::check() const {

	// let's be paranoid
	assert( dofs && size == dofs->getMatrixSize() );

	if( master() ) {
        assert( !notempty(C) );
		assert( !empty(P) );

		// TODO size checks on M, J, ...

	} else {

        if(notempty(C)) {
            assert( C->rows() == int(size) );
		}

	}

	
	return true;
}


void AssemblyVisitor::clear() {

	chunks.clear();
	prefix.clear();
	graph.clear();

}



Visitor::Result AssemblyVisitor::processNodeTopDown(simulation::Node* node) {
    if( !start_node )
    {
        start_node = node;
        isPIdentity = true;
    }

	if( node->mechanicalState ) fill_prefix( node );
    else if( !node->interactionForceField.empty() ) interactionForceField( node );
	return RESULT_CONTINUE;
}

void AssemblyVisitor::processNodeBottomUp(simulation::Node* node) {
	if( node->mechanicalState ) fill_postfix( node );

	// are we finished yo ?
	if( node == start_node ) {

		// backup prefix traversal order
		utils::dfs( graph, prefix_helper( prefix ) );

		// postfix mechanical flags propagation (and geometric stiffness matrices)
        std::for_each(prefix.rbegin(), prefix.rend(), propagation_helper(mparams, graph) );

		// TODO at this point it could be a good thing to prune
		// non-mechanical nodes in the graph, in order to avoid unneeded
		// mapping concatenations, then rebuild the prefix order

		start_node = 0;
	}
}







}
}

