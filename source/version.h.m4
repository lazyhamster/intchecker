m4_include(`version.m4i')m4_dnl
m4_define(`CPP_DEFINE', `#define $1 $2')m4_dnl

CPP_DEFINE(PLUGIN_VERSION_MAJOR, VMAJOR)
CPP_DEFINE(PLUGIN_VERSION_MINOR, VMINOR)
CPP_DEFINE(PLUGIN_VERSION_REVISION, VREVISION)
