#include <dev_config.h>
#include <LiquidCrystal_I2C.h>

class displayLCDHandler
{
private:
    LiquidCrystal_I2C lcd;
    // {'1', '2', ':', '3', '4'};
    String clock = ",,,,,";
    String date = ",,,,,,,,,,";
    // String suhu = "00,0,,";
    String suhu = ",,,,,,,";
    String kelembaban = ",,,,,,,";
    String tipe = ",,,,,";
    String statusPendjadwalan = ",";
    String waktuPendjadwalanTerdekat = ",,,,,";
    String lahan1 = ",";
    String lahan2 = ",";
    String lahan3 = ",";
    String lahan4 = ",";
    String connectionStatus = ",";
    // String kelembaban = "00.0";
    bool stateSecondsTick = false;
    bool isInit = true;
    String baris[4] = {"-:------ -:-----", "-:------ -:-----", "-:------ -:-----", "-:------ -:-----"};

public:
    String makeBarisFrom2String(String text1, String text2, uint8_t startCol1, uint8_t startCol2);
    String makeBarisFromString(String text1, uint8_t startCol);
    void displayBaris(uint8_t baris, String text);
    void initDisplayInformation();
    void initLCD();
    void reinitLCD();
    void displayWifiConnected();
    void displayWifiConnecting();
    void displayWifiNotConnected();
    void displayWifiManagerStarted(String AP_Name, int timeout);
    void displayContinueWithoutWiFi();
    void updateSecondsTick();
    void displayConnectionStatus(String status);
    void displayClock(String date, String time);
    void displaySuhu(String suhu);
    void displayKelembaban(String kelembaban);
    void displayTipe(String tipe);
    void displayStatusPendjadwalan(String status);
    void displayWaktuPendjadwalanTerdekat(String waktu);
    void displayLahan1(String status);
    void displayLahan2(String status);
    void displayLahan3(String status);
    void displayLahan4(String status);
    void setInit(bool isInit);

    displayLCDHandler();
    ~displayLCDHandler();
};
