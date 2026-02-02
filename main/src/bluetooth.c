/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "bluetooth.h"
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "modlog/modlog.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "scli.h"
#include "services/ans/ble_svc_ans.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "util.h"
#include "screen/home.h"
#include "screen/music.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static const char *TAG = "BLUETOOTH";

/*###############################PRIVATE
 * VARIABLES####################################*/
/* A characteristic that can be subscribed to */
static uint8_t gatt_svr_chr_val;
static uint16_t gatt_svr_chr_val_handle;
static const ble_uuid128_t gatt_svr_chr_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22,
                     0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

/* A custom descriptor */
static uint8_t gatt_svr_dsc_val;
static const ble_uuid128_t gatt_svr_dsc_uuid =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12, 0x23, 0x23,
                     0x23, 0x23, 0x34, 0x34, 0x34, 0x34);

static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                     0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* Step Count Characteristic: Write/Read */
static uint16_t gatt_svr_chr_steps_handle;
static const ble_uuid128_t gatt_svr_chr_steps_uuid =
    BLE_UUID128_INIT(0x2e, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                     0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* Song Progress Characteristic: Write/Read */
static uint16_t gatt_svr_chr_progress_handle;
static const ble_uuid128_t gatt_svr_chr_progress_uuid =
    BLE_UUID128_INIT(0x2f, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                     0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* Media Control Characteristic: Notify (ESP32 -> Phone) */
static uint16_t gatt_svr_chr_media_ctrl_handle;
static const ble_uuid128_t gatt_svr_chr_media_ctrl_uuid =
    BLE_UUID128_INIT(0x30, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                     0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* Notification Characteristic: Notify (ESP32 -> Phone) */
static uint16_t gatt_svr_chr_notify_handle;
static const ble_uuid128_t gatt_svr_chr_notify_uuid =
    BLE_UUID128_INIT(0x31, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                     0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

static uint8_t own_addr_type;

static void bleprph_advertise(void);

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    /* Old characteristic preserved for now */
                    .uuid = &gatt_svr_chr_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
                    .val_handle = &gatt_svr_chr_val_handle,
                    .descriptors =
                        (struct ble_gatt_dsc_def[]){
                            {
                                .uuid = &gatt_svr_dsc_uuid.u,
                                .att_flags = BLE_ATT_F_READ,
                                .access_cb = gatt_svc_access,
                            },
                            {
                                0,
                            }},
                },
                {
                    /* Step Count */
                    .uuid = &gatt_svr_chr_steps_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                    .val_handle = &gatt_svr_chr_steps_handle,
                },
                {
                    /* Song Progress */
                    .uuid = &gatt_svr_chr_progress_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                    .val_handle = &gatt_svr_chr_progress_handle,
                },
                {
                    /* Media Control (Notify) */
                    .uuid = &gatt_svr_chr_media_ctrl_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &gatt_svr_chr_media_ctrl_handle,
                },
                {
                    /* General Notification (Notify) */
                    .uuid = &gatt_svr_chr_notify_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &gatt_svr_chr_notify_handle,
                },
                {
                    0, /* No more characteristics in this service. */
                }},
    },

    {
        0, /* No more services. */
    },
};

/*###############################PRIVATE
 * FUNCTIONS####################################*/
void ble_store_config_init(void); // If you don't want bonding, leave empty

static int gatt_svr_write(struct os_mbuf *om, uint16_t min_len,
                          uint16_t max_len, void *dst, uint16_t *len) {
  uint16_t om_len;
  int rc;

  om_len = OS_MBUF_PKTLEN(om);
  if (om_len < min_len || om_len > max_len) {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  }

  rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
  if (rc != 0) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  return 0;
}

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg) {
  const ble_uuid_t *uuid;
  int rc;

  switch (ctxt->op) {
  case BLE_GATT_ACCESS_OP_READ_CHR:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      MODLOG_DFLT(INFO, "Characteristic read; conn_handle=%d attr_handle=%d\n",
                  conn_handle, attr_handle);
    } else {
      MODLOG_DFLT(INFO, "Characteristic read by NimBLE stack; attr_handle=%d\n",
                  attr_handle);
    }
    uuid = ctxt->chr->uuid;
    if (attr_handle == gatt_svr_chr_val_handle) {
      rc =
          os_mbuf_append(ctxt->om, &gatt_svr_chr_val, sizeof(gatt_svr_chr_val));

      ESP_LOGI("NimBLE", "Value read: 0x%02X", gatt_svr_chr_val);
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    goto unknown;

  case BLE_GATT_ACCESS_OP_WRITE_CHR:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      MODLOG_DFLT(INFO, "Characteristic write; conn_handle=%d attr_handle=%d",
                  conn_handle, attr_handle);
    } else {
      MODLOG_DFLT(INFO, "Characteristic write by NimBLE stack; attr_handle=%d",
                  attr_handle);
    }
    uuid = ctxt->chr->uuid;
    if (attr_handle == gatt_svr_chr_val_handle) {
      rc = gatt_svr_write(ctxt->om, sizeof(gatt_svr_chr_val),
                          sizeof(gatt_svr_chr_val), &gatt_svr_chr_val, NULL);
      ESP_LOGI("NimBLE", "Value written: 0x%02X", gatt_svr_chr_val);
      ble_gatts_chr_updated(attr_handle);
      MODLOG_DFLT(INFO, "Notification/Indication scheduled for "
                        "all subscribed peers.\n");
      return rc;
    } else if (attr_handle == gatt_svr_chr_steps_handle) {
        uint32_t steps = 0;
        rc = gatt_svr_write(ctxt->om, sizeof(steps), sizeof(steps), &steps, NULL);
        if (rc == 0) {
            ESP_LOGI("NimBLE", "Steps updated: %" PRIu32, steps);
            home_update_steps(steps);
        }
        return rc;
    } else if (attr_handle == gatt_svr_chr_progress_handle) {
        uint32_t progress = 0;
        rc = gatt_svr_write(ctxt->om, sizeof(progress), sizeof(progress), &progress, NULL);
        if (rc == 0) {
            ESP_LOGI("NimBLE", "Progress updated: %" PRIu32, progress);
            music_update_progress(progress);
        }
        return rc;
    }
    goto unknown;

  case BLE_GATT_ACCESS_OP_READ_DSC:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      MODLOG_DFLT(INFO, "Descriptor read; conn_handle=%d attr_handle=%d\n",
                  conn_handle, attr_handle);
    } else {
      MODLOG_DFLT(INFO, "Descriptor read by NimBLE stack; attr_handle=%d\n",
                  attr_handle);
    }
    uuid = ctxt->dsc->uuid;
    if (ble_uuid_cmp(uuid, &gatt_svr_dsc_uuid.u) == 0) {
      rc =
          os_mbuf_append(ctxt->om, &gatt_svr_dsc_val, sizeof(gatt_svr_chr_val));
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    goto unknown;

  case BLE_GATT_ACCESS_OP_WRITE_DSC:
    goto unknown;

  default:
    goto unknown;
  }

unknown:
  assert(0);
  return BLE_ATT_ERR_UNLIKELY;
}

static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt,
                                 void *arg) {
  char buf[BLE_UUID_STR_LEN];

  switch (ctxt->op) {
  case BLE_GATT_REGISTER_OP_SVC:
    MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                ctxt->svc.handle);
    break;

  case BLE_GATT_REGISTER_OP_CHR:
    MODLOG_DFLT(DEBUG,
                "registering characteristic %s with "
                "def_handle=%d val_handle=%d\n",
                ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                ctxt->chr.def_handle, ctxt->chr.val_handle);
    break;

  case BLE_GATT_REGISTER_OP_DSC:
    MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                ctxt->dsc.handle);
    break;

  default:
    assert(0);
    break;
  }
}

static int gatt_svr_init(void) {
  int rc;

  ble_svc_gap_init();
  ble_svc_gatt_init();
  ble_svc_ans_init();

  rc = ble_gatts_count_cfg(gatt_svr_svcs);
  if (rc != 0) {
    return rc;
  }

  rc = ble_gatts_add_svcs(gatt_svr_svcs);
  if (rc != 0) {
    return rc;
  }

  gatt_svr_dsc_val = 0x99;

  return 0;
}

static void bleprph_print_conn_desc(struct ble_gap_conn_desc *desc) {
  MODLOG_DFLT(INFO,
              "handle=%d our_ota_addr_type=%d our_ota_addr=", desc->conn_handle,
              desc->our_ota_addr.type);
  print_addr(desc->our_ota_addr.val);
  MODLOG_DFLT(INFO,
              " our_id_addr_type=%d our_id_addr=", desc->our_id_addr.type);
  print_addr(desc->our_id_addr.val);
  MODLOG_DFLT(
      INFO, " peer_ota_addr_type=%d peer_ota_addr=", desc->peer_ota_addr.type);
  print_addr(desc->peer_ota_addr.val);
  MODLOG_DFLT(INFO,
              " peer_id_addr_type=%d peer_id_addr=", desc->peer_id_addr.type);
  print_addr(desc->peer_id_addr.val);
  MODLOG_DFLT(INFO,
              " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
              "encrypted=%d authenticated=%d bonded=%d\n",
              desc->conn_itvl, desc->conn_latency, desc->supervision_timeout,
              desc->sec_state.encrypted, desc->sec_state.authenticated,
              desc->sec_state.bonded);
}

static int bleprph_gap_event(struct ble_gap_event *event, void *arg) {
  struct ble_gap_conn_desc desc;
  int rc;

  switch (event->type) {

  case BLE_GAP_EVENT_CONNECT:
    MODLOG_DFLT(INFO, "connection %s; status=%d ",
                event->connect.status == 0 ? "established" : "failed",
                event->connect.status);
    if (event->connect.status == 0) {
      rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
      assert(rc == 0);
      bleprph_print_conn_desc(&desc);

      // /* Force Security Initiation to trigger Pairing on Phone */
      // rc = ble_gap_security_initiate(event->connect.conn_handle);
      // if (rc != 0) {
      //     MODLOG_DFLT(INFO, "Security initiate failed; rc=%d\n", rc);
      // }
    }
    MODLOG_DFLT(INFO, "\n");

    if (event->connect.status != 0) {
      bleprph_advertise();
    }

    return 0;

  case BLE_GAP_EVENT_DISCONNECT:
    MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
    bleprph_print_conn_desc(&event->disconnect.conn);
    MODLOG_DFLT(INFO, "\n");

    bleprph_advertise();
    return 0;

  case BLE_GAP_EVENT_CONN_UPDATE:
    MODLOG_DFLT(INFO, "connection updated; status=%d ",
                event->conn_update.status);
    rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
    assert(rc == 0);
    bleprph_print_conn_desc(&desc);
    MODLOG_DFLT(INFO, "\n");
    return 0;

  case BLE_GAP_EVENT_ADV_COMPLETE:
    MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                event->adv_complete.reason);
    bleprph_advertise();
    return 0;

  case BLE_GAP_EVENT_ENC_CHANGE:
    MODLOG_DFLT(INFO, "encryption change event; status=%d ",
                event->enc_change.status);
    rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
    assert(rc == 0);
    bleprph_print_conn_desc(&desc);
    MODLOG_DFLT(INFO, "\n");
    return 0;

  case BLE_GAP_EVENT_NOTIFY_TX:
    MODLOG_DFLT(INFO,
                "notify_tx event; conn_handle=%d attr_handle=%d "
                "status=%d is_indication=%d",
                event->notify_tx.conn_handle, event->notify_tx.attr_handle,
                event->notify_tx.status, event->notify_tx.indication);
    return 0;

  case BLE_GAP_EVENT_SUBSCRIBE:
    MODLOG_DFLT(INFO,
                "subscribe event; conn_handle=%d attr_handle=%d "
                "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                event->subscribe.conn_handle, event->subscribe.attr_handle,
                event->subscribe.reason, event->subscribe.prev_notify,
                event->subscribe.cur_notify, event->subscribe.prev_indicate,
                event->subscribe.cur_indicate);
    return 0;

  case BLE_GAP_EVENT_MTU:
    MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                event->mtu.conn_handle, event->mtu.channel_id,
                event->mtu.value);
    return 0;

  case BLE_GAP_EVENT_REPEAT_PAIRING:
    rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
    assert(rc == 0);
    ble_store_util_delete_peer(&desc.peer_id_addr);
    return BLE_GAP_REPEAT_PAIRING_RETRY;

  case BLE_GAP_EVENT_PASSKEY_ACTION:
    ESP_LOGI(TAG, "PASSKEY_ACTION_EVENT started");
    struct ble_sm_io pkey = {0};
    int key = 0;

    if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
      pkey.action = event->passkey.params.action;
      pkey.passkey = 123456;
      ESP_LOGI(TAG, "Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
      rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
      ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
    } else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
      ESP_LOGI(TAG, "Passkey on device's display: %" PRIu32,
               event->passkey.params.numcmp);
      ESP_LOGI(TAG, "Accept or reject the passkey through console in this "
                    "format -> key Y or key N");
      pkey.action = event->passkey.params.action;
      if (scli_receive_key(&key)) {
        pkey.numcmp_accept = key;
      } else {
        pkey.numcmp_accept = 0;
        ESP_LOGE(TAG, "Timeout! Rejecting the key");
      }
      rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
      ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
    } else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
      static uint8_t tem_oob[16] = {0};
      pkey.action = event->passkey.params.action;
      for (int i = 0; i < 16; i++) {
        pkey.oob[i] = tem_oob[i];
      }
      rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
      ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
    } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
      ESP_LOGI(TAG,
               "Enter the passkey through console in this format-> key 123456");
      pkey.action = event->passkey.params.action;
      if (scli_receive_key(&key)) {
        pkey.passkey = key;
      } else {
        pkey.passkey = 0;
        ESP_LOGE(TAG, "Timeout! Passing 0 as the key");
      }
      rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
      ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
    }
    return 0;

  case BLE_GAP_EVENT_AUTHORIZE:
    MODLOG_DFLT(INFO,
                "authorize event: conn_handle=%d attr_handle=%d is_read=%d",
                event->authorize.conn_handle, event->authorize.attr_handle,
                event->authorize.is_read);
    event->authorize.out_response = BLE_GAP_AUTHORIZE_REJECT;
    return 0;
  }
  return 0;
}

static void bleprph_advertise(void) {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  const char *name;
  int rc;

  memset(&fields, 0, sizeof fields);
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  name = ble_svc_gap_device_name();
  fields.name = (uint8_t *)name;
  fields.name_len = strlen(name);
  fields.name_is_complete = 1;

  static const ble_uuid16_t adv_uuids[] = {
      BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)};
  fields.uuids16 = adv_uuids;
  fields.num_uuids16 = 1;
  fields.uuids16_is_complete = 1;

  rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
    return;
  }

  memset(&adv_params, 0, sizeof adv_params);
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                         bleprph_gap_event, NULL);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
    return;
  }
}

static void bleprph_on_sync(void) {
  int rc;

  rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
    return;
  }

  uint8_t addr_val[6] = {0};
  rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

  MODLOG_DFLT(INFO, "Device Address: ");
  print_addr(addr_val);
  MODLOG_DFLT(INFO, "\n");
  bleprph_advertise();
}

static void bleprph_on_reset(int reason) {
  MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

// static void bleprph_host_task(void *param) {
//   ESP_LOGI(TAG, "BLE Host Task Started");
//   nimble_port_run();
//   nimble_port_freertos_deinit();
// }
/*###############################PUBLIC
 * FUNCTIONS####################################*/
void bluetooth_main_task(void *param) {
  int rc;
  esp_err_t ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init nimble %d ", ret);
    return;
  }

  ble_hs_cfg.reset_cb = bleprph_on_reset;
  ble_hs_cfg.sync_cb = bleprph_on_sync;
  ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
  ble_hs_cfg.sm_sc = 1;
  ble_hs_cfg.sm_mitm = 1;
  ble_hs_cfg.sm_bonding = 1;

  rc = gatt_svr_init();
  assert(rc == 0);

  rc = ble_svc_gap_device_name_set("nimble-test");
  assert(rc == 0);

  ble_store_config_init();

  ESP_LOGI(TAG, "BLE Host Task Started");
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void bluetooth_send_notification(const char *msg) {
    if (gatt_svr_chr_notify_handle == 0) return;
    
    struct os_mbuf *om = ble_hs_mbuf_from_flat(msg, strlen(msg));
    if (om) {
        ble_gatts_notify_custom(0, gatt_svr_chr_notify_handle, om);
    }
}

void bluetooth_send_media_command(uint8_t cmd) {
    if (gatt_svr_chr_media_ctrl_handle == 0) return;
    
    struct os_mbuf *om = ble_hs_mbuf_from_flat(&cmd, sizeof(cmd));
    if (om) {
        // Handle 0 is usually the first connected peer in simple cases, 
        // but real apps should track active connection handles.
        // For this test, we assume connection handle 1 or similar if active.
        // ble_gatts_notify_custom is for server -> client
        ble_gatts_notify_custom(0, gatt_svr_chr_media_ctrl_handle, om);
    }
}
