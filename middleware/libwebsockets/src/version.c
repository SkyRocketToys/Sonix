#include <generated/snx_mod_version.h>
#include <generated/snx_sdk_conf.h>

#define MODULE_VERSION	"V1.0.0"
#define MODULE(X)	MIDDLEWARE_LIBWEBSOCKETS_##X

static const char hash[] __attribute__ ((__section__ (".snx_version"))) = MODULE(HASH);
static const char time[] __attribute__ ((__section__ (".snx_version"))) = MODULE(TIME);
static const char branch[] __attribute__ ((__section__ (".snx_version"))) = MODULE(BRANCH);
static const char platfrom[] __attribute__ ((__section__ (".snx_version"))) = CONFIG_SYSTEM_PLATFORM;
static const char version[] __attribute__ ((__section__ (".snx_version"))) = MODULE_VERSION;
