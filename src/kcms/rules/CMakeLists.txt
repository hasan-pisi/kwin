# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kwinrules\")
add_definitions(-DKCMRULES)

set(kwinrules_SRCS
    ../../rulebooksettings.cpp
    ../../rules.cpp
    ../../utils/common.cpp
    ../../virtualdesktopsdbustypes.cpp
    kwinsrc.cpp
    optionsmodel.cpp
    ruleitem.cpp
    rulesmodel.cpp
    rulebookmodel.cpp
)

kconfig_add_kcfg_files(kwinrules_SRCS ../../rulesettings.kcfgc)
kconfig_add_kcfg_files(kwinrules_SRCS ../../rulebooksettingsbase.kcfgc)

add_library(KWinRulesObjects STATIC ${kwinrules_SRCS})
set_property(TARGET KWinRulesObjects PROPERTY POSITION_INDEPENDENT_CODE ON)

set(kwin_kcm_rules_XCB_LIBS
    XCB::CURSOR
    XCB::XCB
    XCB::XFIXES
)

set(kcm_libs
    Qt::Quick

    KF6::KCMUtils
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::WindowSystem
    KF6::XmlGui
)

if (KWIN_BUILD_ACTIVITIES)
    set(kcm_libs ${kcm_libs} KF6::Activities)
endif()
target_link_libraries(KWinRulesObjects ${kcm_libs} ${kwin_kcm_rules_XCB_LIBS})

kcoreaddons_add_plugin(kcm_kwinrules SOURCES kcmrules.cpp INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_kwinrules)
target_link_libraries(kcm_kwinrules KWinRulesObjects)

kpackage_install_package(package kcm_kwinrules kcms)
