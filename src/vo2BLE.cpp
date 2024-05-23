

#include "vo2BLE.h"



BoardBLE::BoardBLE() {
    // BoardBLE("EPS32-BLE");
}

BoardBLE::BoardBLE(std::string boardName, String manufacturerName) {
    // This must be called during `setup`
    this->boardName = boardName;
    this->gamePhase = gPhase::init;
    setManufacturerName(manufacturerName);
    BLEDevice::init(boardName);

    allCharProps = (BLECharacteristic::PROPERTY_READ |
                   BLECharacteristic::PROPERTY_WRITE |
                   BLECharacteristic::PROPERTY_NOTIFY |
                   BLECharacteristic::PROPERTY_INDICATE);
}

void BoardBLE::createServer() {

    Serial.println("BLE: create server");

    onGameIndex = NULL;
    onVolume = NULL;
    onConnect = NULL;
    onDisconnect = NULL;
    onGamePhase = NULL;

    pServer = BLEDevice::createServer();

    // Init settings service
    pSettings = pServer->createService(UUID_SER_SETTINGS);
    pBoardName = pSettings->createCharacteristic(UUID_CHAR_BOARDNAME,allCharProps);
    pVolume = pSettings->createCharacteristic(UUID_CHAR_VOLUME,allCharProps);
    pSettings->start();

    // Init game service
    pGame = pServer->createService(UUID_SER_GAME);
    pGameIndex = pGame->createCharacteristic(UUID_CHAR_GAME_INDEX,allCharProps);
    pGameStatus = pGame->createCharacteristic(UUID_CHAR_GAME_PHASE,allCharProps);
    pScoreActual = pGame->createCharacteristic(UUID_CHAR_SCORE_ACTUAL,allCharProps);
    pScoreFinal = pGame->createCharacteristic(UUID_CHAR_SCORE_FINAL,allCharProps);

    pGameData[0] = pGame->createCharacteristic(UUID_CHAR_GAME_DATA_0,allCharProps);
    pGameData[1] = pGame->createCharacteristic(UUID_CHAR_GAME_DATA_1,allCharProps);
    pGameData[2] = pGame->createCharacteristic(UUID_CHAR_GAME_DATA_2,allCharProps);

    pGame->start();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(UUID_SER_SETTINGS);
    pAdvertising->addServiceUUID(UUID_SER_GAME);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    // BLEAdvertisementData advdata = BLEAdvertisementData();
    // std::string servicedata = MANUFACTURE_DATA;
    // advdata.setManufacturerData(servicedata);
    // pAdvertising->setAdvertisementData(advdata);

    // BLEAdvertisementData advdata = BLEAdvertisementData();
    Serial.println("==================seeset =============");
    BLEAdvertisementData advert;
    pAdvertising = pServer->getAdvertising();
    advert.setName("ESP32-new");
    pAdvertising->setAdvertisementData(advert);


    String s;
    int man_code = 0x564f;  // V = 56, O = 4f
    char b2 = (char)(man_code >> 8);
    man_code <<= 8;
    char b1 = (char)(man_code >> 8);
    s.concat(b1);
    s.concat(b2);
    s.concat(this->manufacturerName);
    BLEAdvertisementData pAdvertisementData;
    pAdvertisementData.setManufacturerData(s.c_str());
    Serial.printf("Manufacturer data '%s'\n",s.c_str());
    pAdvertising->setAdvertisementData(pAdvertisementData);


    // Set default values
    pBoardName->setValue(this->boardName);

    // Set callbacks
    pVolume->setCallbacks(new charCallback(&onVolume));
    pGameIndex->setCallbacks(new charCallback(&onGameIndex));
    pGameStatus->setCallbacks(new charCallback(&onGamePhase));

    pServer->setCallbacks(new serverCallback(&onConnect,&onDisconnect));
}

void BoardBLE::setManufacturerName(String name) {
    this->manufacturerName = name;
}

void BoardBLE::startAdvertising() {

    Serial.printf("BLE activated! Starting advertisement... ");

    // BLEDevice::startAdvertising();
    pAdvertising->start();
    Serial.printf("Done!\n");

}

void BoardBLE::stopAdvertising() {

    Serial.printf("Stopping BLE advertisement... ");
    BLEDevice::stopAdvertising();
    Serial.printf("Done!\n");

}


void BoardBLE::setVolume(int volume) {
    Serial.printf("BoardBLE::setVolume(%d) -> ",volume);
    char cstr[16];
    itoa(volume, cstr, 10);
    Serial.printf("setValue -> ");
    pVolume->setValue(cstr);
    Serial.printf("notify -> ");
    pVolume->notify();
    Serial.printf("Done!\n");
}

void BoardBLE::setGameIndex(int idx) {
    char cstr[16];
    itoa(idx, cstr, 10);
    pGameIndex->setValue(cstr);
    pGameIndex->notify();
}

void BoardBLE::setGamePhase(BoardBLE::gPhase status) {
    gamePhase = status;

    char cstr[16];
    itoa((int)status, cstr, 10);
    pGameStatus->setValue(cstr);
    pGameStatus->notify();
}

void BoardBLE::setActualScore(int score) {

    char cstr[16];
    itoa(score, cstr, 10);
    pScoreActual->setValue(cstr);
    pScoreActual->notify();
}

void BoardBLE::setFinalScore(int score) {

    char cstr[16];
    itoa(score, cstr, 10);
    pScoreFinal->setValue(cstr);
    pScoreFinal->notify();
}

void BoardBLE::setGameData(int channel, int value) {

    Serial.printf("BoardBLE::setGameData(%d,%d)\n",channel,value);
    char cstr[16];
    itoa(value, cstr, 10);
    pGameData[channel]->setValue(cstr);
    pGameData[channel]->notify();

    
    // Serial.printf("double: %d\n",sizeof(int));
    // Serial.printf("double: %d\n",sizeof(long));
    // Serial.printf("double: %d\n",sizeof(double));
    // Serial.printf("double: %d\n",sizeof(int64_t));

    // // char buffer[32];
    // char buffer[sizeof(double)];

    // double n = 0;
    // if (channel == 0) {
    //     n = -9223372036854775808;
    // }
    // if (channel == 1) {
    //     n = 6854775808;
    // }
    // if (channel == 2) {
    //     n = 9223372036854775807;
    // }

    // //Transmit
    // memcpy(buffer, &n, sizeof(double));

    // Serial.printf("init >>>%d<<< ",n);
    // // ltoa(n, buffer, 10);
    // Serial.printf("aaa ");
    // for (int i=0; i<sizeof(double); i++) {
    //     Serial.printf("[%d]%d ",i,buffer[i]);
    // }
    // Serial.printf("setValue -> ");
    // // pGameData[channel]->setValue(n);
    // pGameData[channel]->setValue(buffer);
    // Serial.printf("notify -> ");
    // pGameData[channel]->notify();
    // Serial.printf("Done!\n");
}



BoardBLE::charCallback::charCallback(fptr* callback) {

    this->callback = callback;
}

void BoardBLE::charCallback::onRead(BLECharacteristic* pChar) {
    Serial.println("onRead()");
}

void BoardBLE::charCallback::onWrite(BLECharacteristic* pChar) {

    Serial.printf("onWrite(");


    std::__cxx11::string val_str = pChar->getValue();
    int n_bytes = val_str.length();
    char b[n_bytes];
    strcpy(b,val_str.c_str());

    Serial.printf("INT{");
    for (int i=0; i<n_bytes; i++)
    Serial.printf("%d,", b[i]);
    Serial.printf("}");

    Serial.printf(",HEX{");
    for (int i=0; i<n_bytes; i++)
    Serial.printf("%02hhX", b[i]);
    Serial.printf("}");

    Serial.printf(",STR{%s}",val_str.c_str());

    int n = atoi(val_str.c_str());

    Serial.printf(",n=%d)!\n",n);


    if (*callback != NULL) {
        (*callback)(n);     // Same as `callback(n)`
    } else {
        Serial.println("*charCallback: NULL!");
    }

}


BoardBLE::serverCallback::serverCallback(
    fptr* onConnectCallback,
    fptr* onDisconnectCallback
) {
    this->onConnectCallback = onConnectCallback;
    this->onDisconnectCallback = onDisconnectCallback;
}

void BoardBLE::serverCallback::onConnect(BLEServer* pServer) {
    Serial.println("vo2BLE::onConnect()");
    if (*onConnectCallback != NULL) {
        (*onConnectCallback)(0);     // Same as `callback(n)`
    } else {
        Serial.println("*onConnectCallback: NULL!");
    }
}

void BoardBLE::serverCallback::onDisconnect(BLEServer* pServer) {
    Serial.println("vo2BLE::onDisconnect()");
    if (*onDisconnectCallback != NULL) {
        (*onDisconnectCallback)(0);     // Same as `callback(n)`
    } else {
        Serial.println("*onDisconnectCallback: NULL!");
    }
}

