#include <Arduino.h>
#include "Button.h"
#include <NimBLEDevice.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

const int GREEN_LED_PIN = 8;
const int YELLOW_LED_PIN = 9;
const int RED_LED_PIN = 10;

const gpio_num_t BUTTON_PIN = GPIO_NUM_5;

const char* SERVICE_UUID        = "00000000-1111-2222-3333-123456789abc";
const char* COMMAND_CHAR_UUID   = "cccccccc-1111-2222-3333-123456789abc";
const char* EVENT_CHAR_UUID     = "eeeeeeee-1111-2222-3333-123456789abc";

NimBLEService* bleService;
NimBLECharacteristic* commandChar;
NimBLECharacteristic* eventChar;

bool bleServiceRunning = false;
bool bleServiceAdvertising = false;
bool deviceConnected = false;

TFT_eSPI tft = TFT_eSPI();
static lv_color_t buf[284 * 20]; // partial buffer

class ServerCallbacks : public NimBLEServerCallbacks {

  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    deviceConnected = true;
    Serial.println("Client connected");
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override{
    deviceConnected = false;
    Serial.println("Client disconnected");

    NimBLEDevice::startAdvertising();
  }
};

class CommandCallbacks : public NimBLECharacteristicCallbacks {

  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {

    std::string value = pCharacteristic->getValue();

    if(value.length() == 0) return;

    Serial.print("=> Received: ");
    Serial.println(value.c_str());
  }
};

void initBLE() {
  NimBLEDevice::init("BOOKRONO");

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  bleService = server->createService(SERVICE_UUID);

  commandChar = bleService->createCharacteristic(
        COMMAND_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_NR
  );

  eventChar = bleService->createCharacteristic(
        EVENT_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY
  );

  commandChar->setCallbacks(new CommandCallbacks());

  Serial.println("BLE ready");
}

void startAdvertising() {
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
//   advertising->setScanResponse(false);
//   advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
//   advertising->setMinPreferred(0x12);
    advertising->start();
    bleServiceAdvertising = true;
    Serial.println("Started advertising");
}

void sendEvent(String event) {
  if(!deviceConnected) {
    Serial.println("No client connected, cannot send event");
    return;
  };

  eventChar->setValue(event.c_str());
  eventChar->notify();
}

static void onButtonPressDownCb(void *button_handle, void *usr_data) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println("    pressed down");
}

static void onButtonPressUpCb(void *button_handle, void *usr_data) {
    digitalWrite(GREEN_LED_PIN, LOW);
    Serial.println("    pressed up");
}

static void onButtonSingleClickCb(void *button_handle, void *usr_data) {
    Serial.println("--- single click");
    sendEvent("single click");
}

static void onButtonDoubleClickCb(void *button_handle, void *usr_data) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println(">>> double click");
    sendEvent("double click");
    if (!deviceConnected && !NimBLEDevice::getAdvertising()->isAdvertising()) {
        startAdvertising();
    }
}

static void onButtonLongPressStartCb(void *button_handle, void *usr_data) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    Serial.println("+   long press start");
    sendEvent("long press");
}

static void onButtonLongPressUpCb(void *button_handle, void *usr_data) {
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println("+++ long press up");
}

static void onButtonMultipleClickCb(void *button_handle, void *usr_data) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    Serial.println("!!! multiple click");
    sendEvent("triple click");
}

void initLeds() {
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
}

void initButton() {
    Button *btn = new Button(BUTTON_PIN, false);

    btn->attachPressDownEventCb(&onButtonPressDownCb, NULL);
    btn->attachPressUpEventCb(&onButtonPressUpCb, NULL);
    btn->attachSingleClickEventCb(&onButtonSingleClickCb, NULL);
    btn->attachDoubleClickEventCb(&onButtonDoubleClickCb, NULL);
    btn->attachLongPressStartEventCb(&onButtonLongPressStartCb, NULL);
    btn->attachLongPressUpEventCb(&onButtonLongPressUpCb, NULL);
    btn->attachMultipleClickEventCb(&onButtonMultipleClickCb, 3, NULL);
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

void drawInitialUI() {
    lv_obj_t *greetingsLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(greetingsLabel, "Привіт LVGL 9.5.0!");
    lv_obj_set_style_text_font(greetingsLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(greetingsLabel, lv_color_white(), 0);
    lv_obj_align(greetingsLabel, LV_ALIGN_TOP_MID, 0, 8);
}

void initDisplay() {
    tft.init();
    tft.invertDisplay(false);
    tft.setRotation(3);

    lv_init();
    lv_display_t *disp = lv_display_create(284, 76);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, my_disp_flush);

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    drawInitialUI();
}

void setup() {
    Serial.begin(115200);

    initDisplay();
    initLeds();
    initButton();
    initBLE();

    // btn->setParam(button_param_t param, void *value);
    //     BUTTON_LONG_PRESS_TIME_MS = 0,
    //     BUTTON_SHORT_PRESS_TIME_MS,
    //     BUTTON_PARAM_MAX,
}

void loop() {
    lv_timer_handler();
    delay(5);
}