find_package(CMakeRC CONFIG REQUIRED)

cmrc_add_resource_library(mashiro-data
    ALIAS mashiro::data
    NAMESPACE mashiro
    images/Icon.ico    
    shaders/Canvas.vert
    shaders/Canvas.frag
)