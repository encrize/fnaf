#if defined(__APPLE__)
.section __TEXT,__const
.globl _g_embeddedAssetsBundle
.globl _g_embeddedAssetsBundleEnd
_g_embeddedAssetsBundle:
    .incbin "assets.bundle"
_g_embeddedAssetsBundleEnd:
#elif defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
.section .rdata,"dr"
.globl g_embeddedAssetsBundle
.globl g_embeddedAssetsBundleEnd
g_embeddedAssetsBundle:
    .incbin "assets.bundle"
g_embeddedAssetsBundleEnd:
#else
.section .rodata
.globl g_embeddedAssetsBundle
.globl g_embeddedAssetsBundleEnd
g_embeddedAssetsBundle:
    .incbin "assets.bundle"
g_embeddedAssetsBundleEnd:
#endif
