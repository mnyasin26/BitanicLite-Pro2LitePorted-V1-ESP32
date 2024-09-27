#include "header/processSchedule.h"

ArduinoQueue<LandsConfig> landsQueue(QUEUE_SIZE_ITEMS);

outputHandler mainPump(MAIN_PUMP_PIN);
outputHandler waterPump(WATER_PUMP_PIN);
outputHandler nutrientPump(NUTRIENT_PUMP_PIN);
outputHandler lahan1(LAHAN1_PUMP_PIN);
outputHandler lahan2(LAHAN2_PUMP_PIN);
outputHandler lahan3(LAHAN3_PUMP_PIN);
outputHandler lahan4(LAHAN4_PUMP_PIN);
outputHandler buzzer(BUZZER_PIN);
outputHandler ledPenyiraman(LED_PIN_PENYIRAMAN);
outputHandler ledPemupukan(LED_PIN_PEMUPUKAN);

inputHandler input_h;

TimeHandler time_h;
statusHandler status_h;

processSchedule::processSchedule() : nearest(2080, 1, 1, 0, 0, 0)
{
}

void processSchedule::resetQueue()
{
    while (landsQueue.isEmpty() == false)
    {
        landsQueue.dequeue();
    }
}

void processSchedule::buzzerBeep(unsigned long ms)
{
    buzzer.on();
    timerBuzzer = millis();
    while (millis() - timerBuzzer < ms)
    {
        /* code */
    }
    buzzer.off();
}

void processSchedule::processData(String type, StaticJsonDocument<JSON_SIZE> doc)
{

    for (int i = 0; i < doc["jadwal"][type]["days"].size(); i++)
    {
        if (today == doc["jadwal"][type]["days"][i].as<String>())
        {
            for (int j = 0; j < doc["jadwal"][type]["setontimes"].size(); j++)
            {
                // DEBUG_PRINTLN("i: " + (String)i + " j: " + (String)j + " state_water: " + (String)state_water_pump);
                String time_target = doc["jadwal"][type]["setontimes"][j]["time"].as<String>();

                DateTime now = DateTime(2020, 1, 1, time.substring(0, 2).toInt(), time.substring(3, 5).toInt(), time.substring(6, 8).toInt());
                DateTime target = DateTime(2020, 1, 1, time_target.substring(0, 2).toInt(), time_target.substring(3, 5).toInt(), time_target.substring(6, 8).toInt());

                DEBUG_PRINTLN("now: " + time + " target: " + time_target);
                if (now <= target)
                {
                    flag = true;
                    if (target <= nearest)
                    {
                        flag_near = true;
                        DEBUG_PRINTLN("Ada Jadwal Terdekat");
                        /* code */
                        nearest = target;
                    }
                }

                if (flag)
                {
                    if (flag_near)
                    {
                        this->flag_near = false;
                        status_h.setWaktuPendajwalanTerdekatHariIni(time_target.substring(0, 2) + ":" + time_target.substring(3, 5));
                        if (type == "air")
                        {
                            status_h.setTipeTerdekatHariIni("SIRAM");
                            /* code */
                        }
                        else if (type == "pemupukan")
                        {
                            status_h.setTipeTerdekatHariIni("PUPUK");
                        }
                    }
                }

                if (time == time_target)
                {
                    // TODO: set ontypepump
                    if (type == "air")
                    {
                        // DEBUG_PRINTLN("set true");
                        state_water_pump = true;
                        // temp["statusJadwal"]["air"]["setontimes"][j]["status"] = "on";
                        DEBUG_PRINTLN("Waktu untuk menyiram");
                        buzzerBeep(1000);
                        // TODO Handle Error kalau nggak ada land
                    }
                    else if (type == "pemupukan")
                    {
                        state_nutrient_pump = true;
                        buzzerBeep(1000);
                        DEBUG_PRINTLN("Waktu untuk pemupukan");
                    }

                    status_h.setCurrentIndexSetOnTimes(j);
                    lastTime = millis();
                    this->delay_over_lands = doc["jadwal"][type]["setontimes"][j]["delay"].as<String>();
                    for (int k = 0; k < doc["jadwal"][type]["setontimes"][j]["lands"].size(); k++)
                    {
                        DEBUG_PRINTLN("land: " + (String)doc["jadwal"][type]["setontimes"][j]["lands"][k]["id"].as<int>());
                        DEBUG_PRINTLN("duration: " + (String)doc["jadwal"][type]["setontimes"][j]["lands"][k]["duration"].as<int>());
                        LandsConfig temp;
                        StaticJsonDocument<204> test;
                        test = doc["jadwal"][type]["setontimes"][j]["lands"][k];
                        if (type == "air")
                        {

                            temp.tipe = 0;
                            temp.id = test["id"].as<int>();
                            temp.duration = test["duration"].as<int>();
                            temp.seconds = test["seconds"].as<int>();
                            temp.isPemupukan = false;

                            // temp.id = doc["jadwal"][type]["setontimes"][j]["lands"][k]["id"].as<int>();
                            // temp.duration = doc["jadwal"][type]["setontimes"][j]["lands"][k]["duration"].as<int>();
                            // temp.seconds = doc["jadwal"][type]["setontimes"][j]["lands"][k]["seconds"].as<int>();
                            landsQueue.enqueue(temp);
                        }
                        else if (type == "pemupukan")
                        {
                            temp.tipe = 0;
                            temp.id = test["id"].as<int>();
                            temp.duration = test["first_water"]["minutes"].as<int>();
                            temp.seconds = test["first_water"]["seconds"].as<int>();
                            temp.isPemupukan = true;
                            landsQueue.enqueue(temp);

                            // temp.tipe = 0;
                            // temp.id = doc["jadwal"][type]["setontimes"][j]["lands"][k]["id"].as<int>();
                            // temp.duration = doc["jadwal"][type]["setontimes"][j]["lands"][k]["first_water"]["minutes"].as<int>();
                            // temp.seconds = doc["jadwal"][type]["setontimes"][j]["lands"][k]["first_water"]["seconds"].as<int>();
                            // landsQueue.enqueue(temp);

                            // temp.tipe = 1;
                            // temp.id = doc["jadwal"][type]["setontimes"][j]["lands"][k]["id"].as<int>();
                            // temp.duration = doc["jadwal"][type]["setontimes"][j]["lands"][k]["duration"].as<int>();
                            // temp.seconds = doc["jadwal"][type]["setontimes"][j]["lands"][k]["seconds"].as<int>();
                            // landsQueue.enqueue(temp);

                            temp.tipe = 1;
                            temp.id = test["id"].as<int>();
                            temp.duration = test["duration"].as<int>();
                            temp.seconds = test["seconds"].as<int>();
                            temp.isPemupukan = true;
                            landsQueue.enqueue(temp);

                            temp.tipe = 0;
                            temp.id = test["id"].as<int>();
                            temp.duration = test["last_water"]["minutes"].as<int>();
                            temp.seconds = test["last_water"]["seconds"].as<int>();
                            temp.isPemupukan = true;
                            landsQueue.enqueue(temp);

                            // temp.tipe = 0;
                            // temp.id = doc[F("jadwal")][type][F("setontimes")][j][F("lands")][k]["id"].as<int>();
                            // temp.duration = doc[F("jadwal")][type][F("setontimes")][j][F("lands")][k]["last_water"]["minutes"].as<int>();
                            // temp.seconds = doc[F("jadwal")][type][F("setontimes")][j][F("lands")][k]["last_water"]["seconds"].as<int>();
                            // landsQueue.enqueue(temp);
                        }
                    }
                }
            }
        }
    }
}

String temp;

void processSchedule::pumpHandler(String type)
{
    if (isOperate == false && landsQueue.isEmpty() == true)
    {
        if (type == "air")
        {
            state_water_pump = false;
        }
        else if (type == "pemupukan")
        {
            state_nutrient_pump = false;
        }

        status_h.setPenjadwalanOn(false);

        mainPump.off();
    }

    if (isOperate == false && landsQueue.isEmpty() == false)
    {
        current_land = landsQueue.dequeue();
        lastTime = millis();
        isOperate = true;

        status_h.setPenjadwalanOn(true);

        if (current_land.tipe == 0)
        {
            temp = "air";
        }
        else if (current_land.tipe == 1)
        {
            temp = "pemupukan";
        }

        if (current_land.isPemupukan)
        {
            status_h.setOnTimesStatus(true, current_land.id, "pemupukan");
            /* code */
        }
        else
        {
            status_h.setOnTimesStatus(true, current_land.id, temp);
        }

        if (temp == "air")
        {
            status_h.setWaterPump(true);
            DEBUG_PRINTLN("WATER PUMP ON");
            ledPenyiraman.on();
            waterPump.on();
            digitalWrite(LED_PIN_PENYIRAMAN, HIGH);
        }
        else if (temp == "pemupukan")
        {
            // ledPemupukan.on();
            status_h.setNutrientPump(true);
            DEBUG_PRINTLN("NUTRIENT PUMP ON");
            nutrientPump.on();
            digitalWrite(LED_PIN_PENYIRAMAN, HIGH);
        }

        mainPump.on();
        status_h.setMainPump(true);

        DEBUG_PRINTLN("CURRENT LAND: " + (String)current_land.id);
        DEBUG_PRINTLN("DURATION: " + (String)current_land.duration);

        if (current_land.id == 1)
        {
            status_h.setLahan1(true);
            lahan1.on();
        }
        else if (current_land.id == 2)
        {
            status_h.setLahan2(true);
            lahan2.on();
        }
        else if (current_land.id == 3)
        {
            status_h.setLahan3(true);
            lahan3.on();
        }
        else if (current_land.id == 4)
        {
            status_h.setLahan4(true);
            lahan4.on();
        }
    }

    if (isOperate == true && isDelay == false)
    {

        if (millis() - lastTime > (current_land.duration * 1000UL * MINUTE) + 200 + (current_land.seconds * 1000UL))
        {

            if (temp == "air")
            {
                DEBUG_PRINTLN("WATER PUMP OFF");
                status_h.setWaterPump(false);
                ledPenyiraman.off();
                waterPump.off();
                digitalWrite(LED_PIN_PENYIRAMAN, LOW);
            }
            else if (temp == "pemupukan")
            {
                DEBUG_PRINTLN("NUTRIENT PUMP OFF");
                status_h.setNutrientPump(false);
                // ledPemupukan.off();
                nutrientPump.off();
                digitalWrite(LED_PIN_PENYIRAMAN, LOW);
            }

            status_h.setMainPump(false);
            mainPump.off();

            if (current_land.id == 1)
            {
                status_h.setLahan1(false);
                lahan1.off();
            }
            else if (current_land.id == 2)
            {
                status_h.setLahan2(false);
                lahan2.off();
            }
            else if (current_land.id == 3)
            {
                status_h.setLahan3(false);
                lahan3.off();
            }
            else if (current_land.id == 4)
            {
                status_h.setLahan4(false);
                lahan4.off();
            }

            if (landsQueue.isEmpty() == false)
            {
                DEBUG_PRINTLN("DELAY ON");
                timerDelay = millis();
                isDelay = true;
            }
            else
            {
                DEBUG_PRINTLN("DELAY OFF");
                isDelay = false;
                isOperate = false;

                // String temp;
                if (current_land.tipe == 0)
                {
                    temp = "air";
                }
                else if (current_land.tipe == 1)
                {
                    temp = "pemupukan";
                }
                status_h.setOnTimesStatus(false, 0, temp);
            }
        }
    }

    if (isDelay == true)
    {
        if (millis() - timerDelay > delay_over_lands.toInt() * 1000UL)
        {
            DEBUG_PRINTLN("DELAY OFF");
            isDelay = false;
            isOperate = false;
        }
    }
}

void processSchedule::processManualMode(StaticJsonDocument<JSON_SIZE> doc)
{
    bool flag = false;

    if (doc["lahan1"].as<String>() == "on")
    {
        status_h.setLahan1(true);
        lahan1.on();
        flag = true;
    }
    else
    {
        status_h.setLahan1(false);
        lahan1.off();
    }

    if (doc["lahan2"].as<String>() == "on")
    {
        status_h.setLahan2(true);
        lahan2.on();
        flag = true;
    }
    else
    {
        status_h.setLahan2(false);
        lahan2.off();
    }

    if (doc["lahan3"].as<String>() == "on")
    {
        status_h.setLahan3(true);
        lahan3.on();
        flag = true;
    }
    else
    {
        status_h.setLahan3(false);
        lahan3.off();
    }

    if (doc["lahan4"].as<String>() == "on")
    {
        status_h.setLahan4(true);
        lahan4.on();
        flag = true;
    }
    else
    {
        status_h.setLahan4(false);
        lahan4.off();
    }

    if (flag == true)
    {
        status_h.setMainPump(true);
        mainPump.on();
        if (doc["tipe"].as<String>() == "penyiraman")
        {
            status_h.setWaterPump(true);
            waterPump.on();
            digitalWrite(LED_PIN_PENYIRAMAN, HIGH);
            status_h.setNutrientPump(false);
            nutrientPump.off();
        }
        else
        {
            status_h.setWaterPump(false);
            waterPump.off(); 
            status_h.setNutrientPump(true);
            nutrientPump.on();
            digitalWrite(LED_PIN_PENYIRAMAN, HIGH);
        }
    }
    else
    {
        status_h.setMainPump(false);
        status_h.setWaterPump(false);
        status_h.setNutrientPump(false);
        waterPump.off();
        nutrientPump.off();
        mainPump.off();
        digitalWrite(LED_PIN_PENYIRAMAN, LOW);
    }
}

bool processSchedule::getIsSecondsBeat()
{
    if (isDetik == true)
    {
        isDetik = false;
        return true;
    }
    else
    {
        return false;
    }
}

void processSchedule::resetCounter()
{
    counterDetik = 0;
}

bool processSchedule::getIsSeconds2(int seconds)
{
    if (counterDetik2 >= seconds)
    {
        counterDetik2 = 0;
        return true;
    }
    else
    {
        return false;
    }
}

bool processSchedule::getIsSeconds(int seconds)
{
    if (counterDetik >= seconds)
    {
        counterDetik = 0;
        return true;
    }
    else
    {
        return false;
    }
}

void processSchedule::processAutoMode(StaticJsonDocument<JSON_SIZE> doc)
{

    if (millis() - lastSecond > 1000)
    {
        input_h.updateSensor();
        this->today = time_h.getDayofThenWeek();
        this->time = time_h.getTime();
        // this->time = time_h.getTimeHourMinute();
        status_h.setDate(time_h.getDate());
        status_h.setTime(time_h.getTimeHourMinute());
        DEBUG_PRINTLN("Day: " + (String)today + " Time: " + (String)this->time);
        lastSecond = millis();
        // DEBUG_PRINTLN("Waktu: " + (String)((duration.toInt() * 60) - ((lastSecond - lastTime) / 1000) < 0 ? 0 : (duration.toInt() * 60) - ((lastSecond - lastTime) / 1000)));
        DEBUG_PRINTLN("Waktu: " + (String)((lastSecond - lastTime) / 1000));
        DEBUG_PRINTLN("Time Lengkap: " + time_h.getTime());

        DEBUG_PRINTLN("Sisa minggu: " + time_h.getRestOfWeekFrom(doc["date"].as<String>()));

        status_h.setSisaPekan((time_h.getRestOfWeekFrom(doc["date"].as<String>())).toInt());
        status_h.setCurrentPekan((status_h.getTotalPekan() - status_h.getSisaPekan()));
        status_h.setDHTLahan1Temp(input_h.getDHT1Temp());
        status_h.setDHTLahan1Hum(input_h.getDHT1Hum());
        status_h.setDHTLahan2Temp(input_h.getDHT2Temp());
        status_h.setDHTLahan2Hum(input_h.getDHT2Hum());

        DEBUG_PRINTLN(status_h.getTipeTerdekatHariIni());
        DEBUG_PRINTLN(status_h.getWaktuPendajwalanTerdekatHariIni());
        // status_h.updateJSON();
        DEBUG_PRINTLN(status_h.getStatusJSON().memoryUsage());
        counterDetik++;
        counterDetik2++;
        isDetik = true;
        // String temp;
        // serializeJson(status_h.getStatusJSON(), temp);
        // DEBUG_PRINTLN("Data To Be Send: " + temp);
        // DEBUG_PRINTLN("Hari ini ada jadwal");
        this->flag = false;
        this->flag_near = false;

        if (doc["mode"].as<String>() == "auto")
        {
            DEBUG_PRINTLN("Mode Auto");

            // String test[] = doc["jadwal"]["air"]["days"].as<String[]>();

            if (time_h.isBefore(doc["date"].as<String>()))
            {
                DEBUG_PRINTLN("ada jadwal");
                DEBUG_PRINTLN("state_water: " + (String)state_water_pump);
                DEBUG_PRINTLN("state_pupuk: " + (String)state_nutrient_pump);

                if (state_water_pump == false && state_nutrient_pump == false)
                {
                    processData("air", doc);
                    /* code */
                }

                if (state_water_pump == true)
                {
                    pumpHandler("air");
                    /* code */
                }

                if (state_nutrient_pump == false && state_water_pump == false)
                {
                    processData("pemupukan", doc);
                    /* code */
                }

                if (state_nutrient_pump == true)
                {
                    pumpHandler("pemupukan");
                }
            }
            else
            {
                DEBUG_PRINTLN("Sisa minggu sudah habis");
            }
        }

        if (!flag && status_h.getPenjadwalanOn() == false)
        {
            DEBUG_PRINTLN("tidak ada Jadwal Terdekat");
            status_h.setTipeTerdekatHariIni("-----");
            status_h.setWaktuPendajwalanTerdekatHariIni("--:--");
        }

        nearest = DateTime(2080, 1, 1, 0, 0, 0);
    }
}