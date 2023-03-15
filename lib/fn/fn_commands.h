//
// Created by Игорь Данилов on 13.03.2023.
//

#pragma once
/**
 * <a href="https://data.nalog.ru/html/sites/www.new.nalog.ru/docs/kkt/1_2_05_090621.pdf">Команды к ФН</a>
 */
typedef enum {
    FN_CMDGetFNStatus = 0x30,/** Запрос статуса ФН */
    FN_CMDGetFNEndDate = 0x32, /** Запрос срока действия ФН */
    FN_CMDGetFNFwVersion= 0x33, /** Запрос версии ПО ФН */
    FN_CMDGetFFD = 0x3A, /** Запрос версии ФФД */
    FN_CMDGetFNEndDays= 0x3B, /** Запрос оставшегося срока действия ФН */
    FN_CMDGetFNMemoryInfo= 0x3D, /** Запрос ресурса свободной памяти в ФН */
} FN_CMD;
