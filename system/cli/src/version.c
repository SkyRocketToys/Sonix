#include <generated/snx_mod_version.h>
#include <generated/snx_sdk_conf.h>

//V1.0.0 initial version
//V1.0.1 Add CLI tone detection
#define MODULE_VERSION	"V1.0.1"
#define MODULE(X)	CLI_##X

static const char hash[] __attribute__ ((__section__ (".snx_version"))) = MODULE(HASH);
static const char time[] __attribute__ ((__section__ (".snx_version"))) = MODULE(TIME);
static const char branch[] __attribute__ ((__section__ (".snx_version"))) = MODULE(BRANCH);
static const char platfrom[] __attribute__ ((__section__ (".snx_version"))) = CONFIG_SYSTEM_PLATFORM;
static const char version[] __attribute__ ((__section__ (".snx_version"))) = MODULE_VERSION;
