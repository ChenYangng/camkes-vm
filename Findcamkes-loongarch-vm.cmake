#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set(CAMKES_LOONGARCH_VM_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE STRING "")
set(CAMKES_LOONGARCH_VM_HELPERS_PATH "${CMAKE_CURRENT_LIST_DIR}/loongarch_vm_helpers.cmake" CACHE STRING "")
mark_as_advanced(CAMKES_LOONGARCH_VM_DIR CAMKES_LOONGARCH_VM_HELPERS_PATH)

macro(camkes_loongarch_vm_import_project)
    include(${CAMKES_VM_HELPERS_PATH})
    include(${CAMKES_LOONGARCH_VM_HELPERS_PATH})
    # Common build definitions
    CAmkESAddImportPath(${CAMKES_LOONGARCH_VM_DIR}/components camkes-loongarch-vm/components)
    CAmkESAddImportPath(${CAMKES_LOONGARCH_VM_DIR}/interfaces camkes-loongarch-vm/interfaces)
    CAmkESAddTemplatesPath(${CAMKES_LOONGARCH_VM_DIR}/templates camkes-loongarch-vm/templates)
    DeclareCAmkESConnector(
        seL4VMDTBPassthrough
        FROM
        seL4VMDTBPassthrough-from.template.c
        TO
        seL4VMDTBPassthrough-to.template.c
    )

    # Add libraries
    add_subdirectory(${CAMKES_LOONGARCH_VM_DIR}/libs/libvirtio camkes-loongarch-vm/libs/libvirtio)

    # VM components
    add_subdirectory(${CAMKES_LOONGARCH_VM_DIR}/components/VM_LoongArch camkes-loongarch-vm/components/VM)

endmacro()

macro(camkes_loongarch_vm_setup_loongarch_vm_environment)

    find_package(camkes-tool REQUIRED)
    find_package(global-components REQUIRED)
    find_package(camkes-vm REQUIRED)
    find_package(camkes-vm-images REQUIRED)
    find_package(sel4_projects_libs REQUIRED)
    camkes_tool_setup_camkes_build_environment()
    sel4_projects_libs_import_libraries()
    global_components_import_project()

    camkes_loongarch_vm_import_project()
endmacro()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    camkes-loongarch-vm
    DEFAULT_MSG
    CAMKES_LOONGARCH_VM_DIR
    CAMKES_LOONGARCH_VM_HELPERS_PATH
)
