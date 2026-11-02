#include <zephyr/kernel.h>
#include <zephyr/version.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <string.h>
#include <errno.h>
#include "hamlab_version.h"
#include "attenuator.h"
#include "att_codec.h"
#include "network/network.h"
static int cmd_att(const struct shell *sh, size_t argc, char **argv)
{
    int rc = 0;
    if (argc == 3 && !strcmp(argv[1], "set")) {
        unsigned db;
        rc = att_parse_db(argv[2], &db);
        if (!rc) rc = att_set(db);
    } else if (argc != 2 || strcmp(argv[1], "status")) {
        shell_error(sh, "att status | att set <0..127>");
        return -EINVAL;
    }
    if (rc) { shell_error(sh, "att error: %d", rc); return rc; }
    char json[256];
    rc = att_json(json, sizeof(json));
    if (rc < 0) return rc;
    shell_print(sh, "%s", json);
    return 0;
}
static int cmd_info(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    shell_print(sh, "name: pico-zephyr-att-control\nfirmware: " HAMLAB_VERSION
        "\nzephyr: " KERNEL_VERSION_STRING "\nboard: " CONFIG_BOARD
        "\nuptime_s: %lld", (long long)(k_uptime_get()/1000));
    char *status[] = {"att", "status"};
    return cmd_att(sh, 2, status);
}
SHELL_CMD_ARG_REGISTER(att, NULL, "status | set <dB>", cmd_att, 2, 1);
SHELL_CMD_REGISTER(info, NULL, "Firmware and attenuator status", cmd_info);
int main(void)
{
    int rc = att_init();
    printk("pico-zephyr-att-control " HAMLAB_VERSION " GPIO init: %d\n", rc);
    /* Do not delay output initialization while connecting WLAN. */
    int settings_rc = settings_subsys_init();
    if (!settings_rc) settings_rc = settings_load();
    printk("Settings: %d\n", settings_rc);
    /* GP21 is a 32 dB output: never configure it as a boot button.
     * Always start STA; AP remains available via USB: wifi_mode ap. */
    printk("Network boot: %d\n", hamlab_network_boot(false));
    return 0;
}
