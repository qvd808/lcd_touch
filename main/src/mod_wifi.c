#include "mod_wifi.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "MOD_WIFI";

#ifdef SET_ENV
extern const char *WIFI_SSID;
#else
static const char *WIFI_SSID = "";
#endif

void mod_wifi_init(void) {
	ESP_LOGI(TAG, "Initialize WiFi module");

	const char *wifi_ssid = WIFI_SSID;

	// Check if WIFI_SSID is empty
	if (strlen(wifi_ssid) == 0) {
	ESP_LOGW(TAG, "WIFI_SSID environment variable not set or empty");
	return;
	}

	ESP_LOGI(TAG, "WiFi SSID: %s", wifi_ssid);

	// TODO: Add actual WiFi initialization code here
	return;
}
