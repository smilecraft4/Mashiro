find_package(CMakeRC CONFIG REQUIRED)

cmrc_add_resource_library(mashiro-data
    ALIAS mashiro::data
    NAMESPACE mashiro
    images/icon.ico    
    images/icon_16x16.png
    images/icon_32x32.png
    images/icon_48x48.png
    shaders/Canvas.vert
    shaders/Canvas.frag
    shaders/Brush.comp
)