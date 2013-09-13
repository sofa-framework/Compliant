include(${SOFA_CMAKE_DIR}/preProject.cmake)

set(HEADER_FILES

	initCompliant.h
    CompliantAttachButtonSetting.h
    UniformCompliance.h
    UniformCompliance.inl
    ComplianceSolver.h
	DampingCompliance.h
	RigidMass.h
#	UnilateralMask.h
	RigidJointMapping.h
	AssembledRigidRigidMapping.h
	KKTSolver.h
	LDLTSolver.h
  MinresSolver.h
	KrylovSolver.h
	CgSolver.h
	Response.h
	LDLTResponse.h
	DiagonalResponse.h
    AssemblyHelper.h
    AssemblyVisitor.h
	AssembledSystem.h
	AssembledSolver.h
	AssembledMapping.h
	Stabilization.h
    CompliantSolverMerger.h
	
      utils/cast.h
      utils/cg.h
      utils/find.h
      utils/force_assert.h
      utils/graph.h
      utils/krylov.h
      utils/minres.h
      utils/pair.h
      utils/scoped.h
      utils/se3.h
      utils/sparse.h
      utils/thread_local.h
	)

set(SOURCE_FILES

	initCompliant.cpp
    CompliantAttachButtonSetting.cpp
    UniformCompliance.cpp
    DiagonalCompliance.cpp
    ComplianceSolver.cpp
	DampingCompliance.cpp
	RigidMass.cpp
	RigidJointMapping.cpp
	AssembledRigidRigidMapping.cpp
	LDLTSolver.cpp
    MinresSolver.cpp
		KrylovSolver.cpp
		CgSolver.cpp
		LDLTResponse.cpp
		DiagonalResponse.cpp
	AssemblyVisitor.cpp
	AssembledSystem.cpp
	AssembledSolver.cpp
	Stabilization.cpp 
    CompliantSolverMerger.cpp
	)

set(README_FILES

	Compliant.txt
	)

if(WIN32)
	include_directories("${SOFA_EXTLIBS_DIR}/SuiteSparse/cholmod/Include")
	link_directories("${SOFA_EXTLIBS_DIR}/SuiteSparse/cholmod/Lib")
else()
	include_directories("/usr/include/suitesparse/")
endif()
	
add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${README_FILES})

AddCompilerDefinitions("SOFA_BUILD_Compliant")
AddLinkerDependencies(SofaEigen2Solver SofaUserInteraction SofaComponentMisc SofaHelper SofaSimulationGraph amd lapack blas camd colamd cholmod)

# WIN32: copy the needed dlls from the lib dir to the Sofa bin dir
if(WIN32)
	file(GLOB sharedObjects "${SOFA_EXTLIBS_DIR}/SuiteSparse/cholmod/Lib/*.dll")
	foreach(sharedObject ${sharedObjects})
		file(COPY ${sharedObject} DESTINATION "${SOFA_BIN_DIR}")
	endforeach()
endif()
	
include(${SOFA_CMAKE_DIR}/postProject.cmake)
include(custom.cmake OPTIONAL)