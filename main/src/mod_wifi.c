#include "mod_wifi.h"
#include "esp_log.h"
#include <string.h>  // Added for strlen()

/* DEFINES */
// WIFI_SSID is now defined at compile time from .env file
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

// Convert macro to string if it's not already a string literal
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Use this if WIFI_SSID might not be a string literal
static const char* wifi_ssid = WIFI_SSID;

static const char *TAG = "MOD_WIFI";

void mod_wifi_init(void) {
	ESP_LOGI(TAG, "Initialize WiFi module");
	
	// Check if WIFI_SSID is empty
	if (strlen(wifi_ssid) == 0) {
		ESP_LOGW(TAG, "WIFI_SSID environment variable not set or empty");
		return;
	}
	
	ESP_LOGI(TAG, "WiFi SSID: %s", wifi_ssid);
	
	// TODO: Add actual WiFi initialization code here
	
	return;
}
