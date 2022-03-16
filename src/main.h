#ifndef MAIN_H /* include guards */
#define MAIN_H

#include <Arduino.h>

struct UsageMsg
{
    String src;
    int dsb;
    String time;
    float tmpr;
    String sensor_id;
    int sensor_tp;
    int ch1_watts;
    int ch2_watts;
    float price;
    float cost;
};

void handleMessage(String data);

void handleRegularMessage(String data);

void handleHistoryMessage(String data);

void firebase_test(UsageMsg msg);
void firebase_write(UsageMsg msg);

void parseMessageDataAsXml(String data);

void parseMessageDataAsRegex(String data);

char *StringToChar(String str);


void evaluate(bool action);

String xml_parse(String inStr,String needParam, int& startIndex);

void debug_msg(UsageMsg msg);

void doAnomolyDetection(UsageMsg *pMsg);
#endif /* MAIN_H */
