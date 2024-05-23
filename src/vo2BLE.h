
#ifndef VO2BLE
#define VO2BLE

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// https://www.guidgenerator.com/

/** Service which contains settings characteristics */
#define UUID_SER_SETTINGS       "ca45e9af-6cea-483d-b7fa-6912bb84af6d"
/** Settings -> Board name */
#define UUID_CHAR_BOARDNAME     "48f5b82a-8456-4b25-9f1b-29a66e5f205a"
/** Settings -> Volume */
#define UUID_CHAR_VOLUME        "89a6172f-69ad-4f3f-8f3d-853c209c3d5e"

/** Service which contains information about selected game */
#define UUID_SER_GAME           "fb15f92f-b758-495e-b16f-35d2ead65138"
#define UUID_CHAR_GAME_INDEX    "fb33d8cc-3ee8-4dd2-8214-f656350a0a6b"
#define UUID_CHAR_GAME_PHASE    "4aa41949-5c22-4d54-ab19-98de861d244c"
#define UUID_CHAR_SCORE_ACTUAL  "d0b6d22d-70ba-4631-8651-b5fc34ff4fa9"
#define UUID_CHAR_SCORE_FINAL   "3f0c7760-bfdd-4b82-af01-01175a4d8c8f"

#define UUID_CHAR_GAME_DATA_0   "76731b8b-9c6a-4396-a329-55f861351da1"
#define UUID_CHAR_GAME_DATA_1   "c8cbb10b-c281-4375-9f93-8cdd618fad3d"
#define UUID_CHAR_GAME_DATA_2   "f4dcdec6-6622-4e30-b36e-3af24847c45d"

#define MANUFACTURE_DATA        "c119f481-7546-4bbd-99c2-f209f761e80b"

/**
 */
class BoardBLE {

public:

    enum class gPhase {
        init,
        starting,           // Show starting animation
        playing,            // Play
        gameOver,
    };

    // Use this typedef instead of using `void (*funcName)(int);`
    typedef void (*fptr)(int);
    fptr onConnect;
    fptr onDisconnect;
    fptr onVolume;
    fptr onGameIndex;
    fptr onGamePhase;

    /**
     * Empty constructor used for arrays of object
     * Example: `BoardBLE boards_ble[2];`
    */
    BoardBLE();

    /**
     * @brief Construct a new Board BLE
     * @param boardName Name of the board which appears on searched BLE list
     */
    BoardBLE(std::string boardName, String manufacturerName);

    void createServer();

    void setManufacturerName(String name);

    void startAdvertising();
    void stopAdvertising();


    void setGameIndex(int idx);
    void setGamePhase(BoardBLE::gPhase status);
    void setActualScore(int score);
    void setFinalScore(int score);
    void setGameData(int channel, int value);

    void setVolume(int volume);

    std::string boardName;

    /** New volume was detected */
    volatile bool newVolume = false;
    /** Value of new volume pending to be applied */
    volatile int volume = 0;
    /** Write volume to NVS when this constant is written to volume char */
    static const int VOLUME_WRITE = 101;

    /** New game index detected */
    volatile bool newGameIndex = false;
    /** Value of new game index pending to be applied */
    volatile int gameIndex = 0;

    /** New game phase detected */
    volatile bool newGamePhase = false;
    /** Value of new game phase pending ot be applied */
    volatile gPhase gamePhase;


private:

    BLEServer *pServer;

    BLEAdvertising *pAdvertising;

    BLEService *pSettings;
    BLECharacteristic *pBoardName;
    BLECharacteristic *pVolume;

    BLEService *pGame;
    BLECharacteristic *pGameIndex;
    BLECharacteristic *pGameStatus;
    BLECharacteristic *pScoreActual;
    BLECharacteristic *pScoreFinal;
    BLECharacteristic *pGameData[3];

    String manufacturerName;

    uint32_t allCharProps;


    class charCallback: public BLECharacteristicCallbacks {
        void onRead(BLECharacteristic* pChar);
        void onWrite(BLECharacteristic* pChar);

    public:
        charCallback(fptr* callback);

    private:
        fptr *callback;

    };

    class serverCallback: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);
    
    public:
        serverCallback(fptr* onConnectCallback, fptr* onDisconnectCallback);

    private:
        fptr* onConnectCallback;
        fptr* onDisconnectCallback;
    };

};

#endif
