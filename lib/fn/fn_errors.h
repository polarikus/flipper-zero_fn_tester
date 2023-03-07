//
// Created by Игорь Данилов on 07.03.2023.
//

#pragma once

typedef enum {
    //Универсальные коды ответов ФН
    /**Успешное выполнение команды*/
    FNOk = 0x00,
    /**Неизвестная команда, неверный формат посылки или неизвестные параметры*/
    FNErrorUnknownCMD = 0x01,
    //Специальные коды ответов ФН

    /**Другое состояние ФН*/
    FNErrorWrongState = 0x02,
    /**Отказ ФН*/
    FNErrorFailure = 0x03,
    /**Отказ КС*/
    FNErrorFailureKS = 0x04,
    /**Параметры команды не соответствуют сроку жизни ФН*/
    FNErrorCMDParamsNotMatchFNEndDate = 0x05,
    /**Некорректная дата и/или время*/
    FNErrorWrongDateTime = 0x07,
    /**Нет запрошенных данных*/
    FNErrorNoRequestedData = 0x08
} FNError;//TODO Заполнить все ошибки (Нудно, долго, для бета теста этих пока хватит)