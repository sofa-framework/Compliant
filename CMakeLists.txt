cmake_minimum_required(VERSION 3.12)
project(Compliant VERSION 0.1)

sofa_find_package(SofaSimulationGraph REQUIRED)
find_package(SofaMiscMapping REQUIRED)
find_package(SofaMiscCollision REQUIRED)
find_package(SofaGuiQt QUIET)
sofa_find_package(SofaPython QUIET)
sofa_find_package(Sofa.GL QUIET)

# Previous Eigen versions have a critical bug (v.noalias()+=w does not work in some situations)
find_package(Eigen3 3.2.5 REQUIRED)

set(HEADER_FILES
    config.h.in
    assembly/AssembledSystem.h
    assembly/AssemblyHelper.h
    assembly/AssemblyVisitor.h
    compliance/DampingCompliance.h
    compliance/DiagonalCompliance.h
    compliance/DiagonalCompliance.inl
    compliance/LinearDiagonalCompliance.h
    compliance/LinearDiagonalCompliance.inl
    compliance/UniformCompliance.h
    compliance/UniformCompliance.inl
    compliance/FullCompliance.h
    compliance/FullCompliance.inl
    constraint/BaseConstraintValue.h
    constraint/ClosureConstraint.h
    constraint/Constraint.h
    constraint/ConstraintValue.h
    constraint/CoulombConstraint.h
    constraint/CoulombConstraint.inl
    constraint/DampingValue.h
    constraint/HolonomicConstraintValue.h
    constraint/OffsettedConstraintValue.h
    constraint/ResistanceConstraint.h
    constraint/Restitution.h
    constraint/Stabilization.h
    constraint/UnilateralConstraint.h
    constraint/VelocityConstraintValue.h
    constraint/BaumgarteStabilization.h
    contact/BaseContact.h
    contact/CompliantContact.h
    contact/FrictionCompliantContact.h
    contact/PenaltyCompliantContact.h
    controller/CompliantSleepController.h
    forcefield/PotentialEnergy.h
    forcefield/UniformLinearPotentialEnergy.h
    forcefield/CompliantPenaltyForceField.h
    forcefield/UniformStiffness.h
    forcefield/UniformStiffness.inl
    forcefield/DiagonalStiffness.h
    forcefield/DiagonalStiffness.inl
    mapping/AdditionMapping.h
    mapping/AffineMultiMapping.h
    mapping/AssembledMapping.h
    mapping/AssembledMultiMapping.h
    mapping/AssembledRigidRigidMapping.h
    mapping/ContactMapping.h
    mapping/ConstantAssembledMapping.h
    mapping/ConstantAssembledMultiMapping.h
    mapping/ContactMultiMapping.h
    mapping/DifferenceFromTargetMapping.h
    mapping/DifferenceMapping.h
    mapping/DotProductMapping.h
    mapping/GearMapping.h
    mapping/MaskMapping.h
    mapping/NormalizationMapping.h
    mapping/PairingMultiMapping.h
    mapping/ProjectionMapping.h
    mapping/QuadraticMapping.h
    mapping/RigidComMultiMapping.h
    mapping/RigidJointMapping.h
    mapping/RigidRestJointMapping.h
    mapping/RigidJointMultiMapping.h
    mapping/RigidJointFromTargetMapping.h
    mapping/SafeDistanceMapping.h
    mapping/WinchMultiMapping.h
    misc/CompliantAttachButtonSetting.h
    misc/CompliantSolverMerger.h
    misc/FailNode.h
    misc/RigidMass.h
    misc/SimpleAnimationLoop.h
    numericalsolver/AnalysisSolver.h
    numericalsolver/Benchmark.h
    numericalsolver/BenchmarkSolver.h
    numericalsolver/BiCgStabSolver.h
    numericalsolver/CgSolver.h
    numericalsolver/DiagonalResponse.h
    numericalsolver/EigenSparseResponse.h
    numericalsolver/EigenSparseResponse.inl
    numericalsolver/EigenSparseSolver.h
    numericalsolver/EigenSparseSolver.inl
    numericalsolver/IterativeSolver.h
    numericalsolver/KKTSolver.h
    numericalsolver/KrylovSolver.h
    numericalsolver/LumpedResponse.h
    numericalsolver/MinresSolver.h
    numericalsolver/ModulusSolver.h
    numericalsolver/NNCGSolver.h
    numericalsolver/PreconditionedCgSolver.h
    numericalsolver/PreconditionedSolver.h
    numericalsolver/Response.h
    numericalsolver/SequentialSolver.h
    numericalsolver/SubKKT.h
    numericalsolver/SubKKT.inl
    odesolver/CompliantImplicitSolver.h
    odesolver/CompliantNLImplicitSolver.h
    odesolver/CompliantPostStabilizationAnimationLoop.h
    odesolver/CompliantPseudoStaticSolver.h
    odesolver/CompliantPseudoStaticSolver.inl
    odesolver/CompliantStaticSolver.h
    odesolver/ConstantCompliantImplicitSolver.h
    preconditioner/BasePreconditioner.h
    preconditioner/CompliantJacobiPreconditioner.h
    preconditioner/CompliantLDLTPreconditioner.h
    preconditioner/IncompleteCholeskyPreconditioner.h
    utils/anderson.h
    utils/nlnscg.h
    utils/basis.h
    utils/bicgstab.h
    utils/cg.h
    utils/cone.h
    utils/edit.h
    utils/find.h
    utils/force_assert.h
    utils/graph.h
    utils/kkt.h
    utils/krylov.h
    utils/map.h
    utils/minres.h
    utils/nan.h
    utils/preconditionedcg.h
    utils/schur.h
    utils/scoped.h
    utils/se3.h
    utils/sparse.h
    utils/sub_kkt.h
    utils/sub_kkt.inl
    utils/thread_variable.h
    )
set(SOURCE_FILES
    assembly/AssembledSystem.cpp
    assembly/AssemblyVisitor.cpp
    compliance/DampingCompliance.cpp
    compliance/DiagonalCompliance.cpp
    compliance/LinearDiagonalCompliance.cpp
    compliance/UniformCompliance.cpp
    compliance/FullCompliance.cpp
    constraint/ClosureConstraint.cpp
    constraint/Constraint.cpp
    constraint/ConstraintValue.cpp
    constraint/CoulombConstraint.cpp
    constraint/DampingValue.cpp
    constraint/HolonomicConstraintValue.cpp
    constraint/OffsettedConstraintValue.cpp
    constraint/ResistanceConstraint.cpp
    constraint/Restitution.cpp
    constraint/Stabilization.cpp
    constraint/UnilateralConstraint.cpp
    constraint/VelocityConstraintValue.cpp
    constraint/BaumgarteStabilization.cpp
    contact/CompliantContact.cpp
    contact/FrictionCompliantContact.cpp
    contact/PenaltyCompliantContact.cpp
    controller/CompliantSleepController.cpp
    forcefield/PotentialEnergy.cpp
    forcefield/UniformLinearPotentialEnergy.cpp
    forcefield/CompliantPenaltyForceField.cpp
    forcefield/UniformStiffness.cpp
    forcefield/DiagonalStiffness.cpp
    initCompliant.cpp
    mapping/AdditionMapping.cpp
    mapping/AffineMultiMapping.cpp
    mapping/AssembledRigidRigidMapping.cpp
    mapping/ContactMapping.cpp
    mapping/ContactMultiMapping.cpp
    mapping/DifferenceFromTargetMapping.cpp
    mapping/DifferenceMapping.cpp
    mapping/DotProductMapping.cpp
    mapping/GearMapping.cpp
    mapping/MaskMapping.cpp
    mapping/NormalizationMapping.cpp
    mapping/PairingMultiMapping.cpp
    mapping/ProjectionMapping.cpp
    mapping/QuadraticMapping.cpp
    mapping/RigidComMultiMapping.cpp
    mapping/RigidJointMapping.cpp
    mapping/RigidRestJointMapping.cpp
    mapping/RigidJointMultiMapping.cpp
    mapping/RigidJointFromTargetMapping.cpp
    mapping/SafeDistanceMapping.cpp
    mapping/WinchMultiMapping.cpp
    misc/CompliantAttachButtonSetting.cpp
    misc/CompliantSolverMerger.cpp
    misc/FailNode.cpp
    misc/RigidMass.cpp
    misc/SimpleAnimationLoop.cpp
    numericalsolver/AnalysisSolver.cpp
    numericalsolver/Benchmark.cpp
    numericalsolver/BenchmarkSolver.cpp
    numericalsolver/BiCgStabSolver.cpp
    numericalsolver/CgSolver.cpp
    numericalsolver/DiagonalResponse.cpp
    numericalsolver/EigenSparseResponse.cpp
    numericalsolver/EigenSparseSolver.cpp
    numericalsolver/IterativeSolver.cpp
    numericalsolver/KrylovSolver.cpp
    numericalsolver/LumpedResponse.cpp
    numericalsolver/MinresSolver.cpp
    numericalsolver/ModulusSolver.cpp
    numericalsolver/NNCGSolver.cpp
    numericalsolver/PreconditionedCgSolver.cpp
    numericalsolver/PreconditionedSolver.cpp
    numericalsolver/SequentialSolver.cpp
    numericalsolver/SubKKT.cpp
    odesolver/CompliantImplicitSolver.cpp
    odesolver/CompliantNLImplicitSolver.cpp
    odesolver/CompliantPostStabilizationAnimationLoop.cpp
    odesolver/CompliantPseudoStaticSolver.cpp
    odesolver/CompliantStaticSolver.cpp
    odesolver/ConstantCompliantImplicitSolver.cpp
    preconditioner/CompliantJacobiPreconditioner.cpp
    preconditioner/CompliantLDLTPreconditioner.cpp
    preconditioner/IncompleteCholeskyPreconditioner.cpp
    utils/anderson.cpp
    utils/nlnscg.cpp
    utils/sub_kkt.cpp
    )
set(README_FILES
    Compliant.txt
    )
if(SofaPython_FOUND)
    set(PYTHON_FILES
        python/Compliant/Control.py
        python/Compliant/Frame.py
        python/Compliant/Rigid.py
        python/Compliant/Script.py
        python/Compliant/StructuralAPI.py
        python/Compliant/Tools.py
        python/Compliant/Vec.py
        python/Compliant/__init__.py
        python/Compliant/future.py
        python/Compliant/sml.py
        )
    list(APPEND HEADER_FILES
        mapping/PythonMultiMapping.h
        mapping/PythonMultiMapping.inl
        python/Binding_AssembledSystem.h
        python/python.h
        )
    list(APPEND SOURCE_FILES
        mapping/PythonMultiMapping.cpp
        python/_Compliant.cpp
        python/Binding_AssembledSystem.cpp

        # note: need to fix assembled solver in python before removing
        python/python.cpp
        )

    # Config files and install rules for pythons scripts
    sofa_install_pythonscripts(PLUGIN_NAME ${PROJECT_NAME} PYTHONSCRIPTS_SOURCE_DIR "python")
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${README_FILES} ${PYTHON_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSOFA_BUILD_${PROJECT_NAME}")
target_link_libraries(${PROJECT_NAME} PUBLIC SofaEigen2Solver SofaUserInteraction SofaMiscMapping SofaSimulationGraph SofaMiscCollision)
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

# linking explicitly with rt looks necessary on some posix (e.g. CentOS)
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE rt)
endif()

if(SofaPython_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC SofaPython)
endif()

if(Sofa.GL_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC Sofa.GL)
endif()

if(SOFA_BUILD_TESTS)
    find_package(SofaTest QUIET)
    if(SofaTest_FOUND)
        add_subdirectory(Compliant_test)
    endif()
endif()

if(SofaGuiQt_FOUND)
    add_subdirectory(Compliant_gui)
endif()

## Install rules for the library; CMake package configurations files
sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    RELOCATABLE "plugins"
    )
