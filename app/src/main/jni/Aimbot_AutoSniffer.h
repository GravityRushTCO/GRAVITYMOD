// AUTO-GENERATED DATAGRAM SNIFFERS

#include "Includes/Logger.h"

#include "Dobby/dobby.h"

#include <string>

#include <stdio.h>

#include <android/log.h>



#ifndef AB_LOGI

#define AB_LOG_TAG "AIMBOT"

#define AB_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  AB_LOG_TAG, __VA_ARGS__)

#endif



static std::string format_datagram_fields(void* _this, int dg_id) {
    char buf[1024];
    switch(dg_id) {
        case 0: {
            snprintf(buf, sizeof(buf), "Type=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4));
            return std::string(buf);
        }
        case 1: {
            snprintf(buf, sizeof(buf), "Type=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 2: {
            snprintf(buf, sizeof(buf), "Type=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4));
            return std::string(buf);
        }
        case 3: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 4: {
            snprintf(buf, sizeof(buf), "VehicleId=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 5: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 6: {
            snprintf(buf, sizeof(buf), "CategoryType=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4));
            return std::string(buf);
        }
        case 7: {
            snprintf(buf, sizeof(buf), "UnviewedDailyTasks=%d | AvailableDailyRewards=%d | UnviewedWeeklyTasks=%d | AvailableWeeklyRewards=%d | UnviewedTutorialTasks=%d | AvailableTutorialRewards=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xc), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x14));
            return std::string(buf);
        }
        case 8: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 9: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 10: {
            snprintf(buf, sizeof(buf), "Controller=%d | Target=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 11: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 12: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 13: {
            snprintf(buf, sizeof(buf), "VehicleId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 14: {
            snprintf(buf, sizeof(buf), "Type=%d | RemoteId=%d | HasModel=%d | Model=%d | HasDisplacement=%d | Position=(%.1f,%.1f,%.1f) | Rotation=(%.1f,%.1f,%.1f) | HasHealth=%.2f | Health=%.2f | HasRadius=%.2f | Radius=%.2f", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(bool*)((char*)_this + 0xc), *(int*)((char*)_this + 0x10), *(bool*)((char*)_this + 0x14), *(float*)((char*)_this + 0x18), *(float*)((char*)_this + 0x1c), *(float*)((char*)_this + 0x20), *(float*)((char*)_this + 0x24), *(float*)((char*)_this + 0x28), *(float*)((char*)_this + 0x2c), *(float*)((char*)_this + 0x34), *(float*)((char*)_this + 0x38), *(float*)((char*)_this + 0x3c), *(float*)((char*)_this + 0x40));
            return std::string(buf);
        }
        case 15: {
            snprintf(buf, sizeof(buf), "Id=%d | Position=(%.1f,%.1f,%.1f) | Rotation=(%.1f,%.1f,%.1f) | Health=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4), *(float*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10), *(float*)((char*)_this + 0x14), *(float*)((char*)_this + 0x18), *(float*)((char*)_this + 0x20));
            return std::string(buf);
        }
        case 16: {
            snprintf(buf, sizeof(buf), "Label=%d | Icon=%d | Atlas=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 17: {
            snprintf(buf, sizeof(buf), "CurrentLatency=%d | RequestTime=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 18: {
            snprintf(buf, sizeof(buf), "RequestTime=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 19: {
            snprintf(buf, sizeof(buf), "Element=%d | Player=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 20: {
            snprintf(buf, sizeof(buf), "Id=%d | IsLocal=%d", *(int*)((char*)_this + 0x0), *(bool*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 21: {
            snprintf(buf, sizeof(buf), "Value=%.2f", *(float*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 22: {
            snprintf(buf, sizeof(buf), "EndTime=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 23: {
            snprintf(buf, sizeof(buf), "SentMoney=%d | ReceivedMoney=%d | LastSendingTime=%d | LastUpdate=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x18));
            return std::string(buf);
        }
        case 24: {
            snprintf(buf, sizeof(buf), "Value=%.2f", *(float*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 25: {
            snprintf(buf, sizeof(buf), "State=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 26: {
            snprintf(buf, sizeof(buf), "StartTime=%d | ItemsPurchased=%d | RemoveComponent=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(int*)((char*)_this + 0x5));
            return std::string(buf);
        }
        case 27: {
            snprintf(buf, sizeof(buf), "PlayerId=%d | VehiclesIds=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 28: {
            snprintf(buf, sizeof(buf), "Value=%.2f", *(float*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 29: {
            snprintf(buf, sizeof(buf), "TutorIsActive=%d | ActionLockButThis=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 30: {
            snprintf(buf, sizeof(buf), "Id=%d | Dimension=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 31: {
            snprintf(buf, sizeof(buf), "Id=%d | Dimension=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 32: {
            snprintf(buf, sizeof(buf), "Id=%d | HipsPos=(%.1f,%.1f,%.1f) | HipsRotation=(%.1f,%.1f,%.1f) | SpinePos=(%.1f,%.1f,%.1f) | SpineRotation=(%.1f,%.1f,%.1f)", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4), *(float*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10), *(float*)((char*)_this + 0x14), *(float*)((char*)_this + 0x18), *(float*)((char*)_this + 0x20), *(float*)((char*)_this + 0x24), *(float*)((char*)_this + 0x28), *(float*)((char*)_this + 0x2c), *(float*)((char*)_this + 0x30), *(float*)((char*)_this + 0x34));
            return std::string(buf);
        }
        case 33: {
            snprintf(buf, sizeof(buf), "TotalSeconds=%d | SecondDuration=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 34: {
            snprintf(buf, sizeof(buf), "Id=%d | Type=%d | Value1=%.2f | Value2=%.2f", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 35: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 36: {
            snprintf(buf, sizeof(buf), "VehicleRemoteId=%d | DoorIndex=%d | IsOpen=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(bool*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 37: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 38: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 39: {
            snprintf(buf, sizeof(buf), "ViolationCount=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 40: {
            snprintf(buf, sizeof(buf), "playerId=%d | weaponItem=%d | point=%d | direction=(%.1f,%.1f,%.1f)", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(float*)((char*)_this + 0x1c), *(float*)((char*)_this + 0x20), *(float*)((char*)_this + 0x24));
            return std::string(buf);
        }
        case 41: {
            snprintf(buf, sizeof(buf), "playerId=%d | point=%d | direction=(%.1f,%.1f,%.1f) | damage=%.2f | fireRate=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(float*)((char*)_this + 0x10), *(float*)((char*)_this + 0x14), *(float*)((char*)_this + 0x18), *(float*)((char*)_this + 0x1c), *(int*)((char*)_this + 0x20));
            return std::string(buf);
        }
        case 42: {
            snprintf(buf, sizeof(buf), "Id=%d | Index=%d | State=%d | Animate=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xa), *(int*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 43: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 44: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 45: {
            snprintf(buf, sizeof(buf), "Id=%d | Type=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 46: {
            snprintf(buf, sizeof(buf), "Id=%d | ColorGroupType=%d | Color=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 47: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 48: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 49: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 50: {
            snprintf(buf, sizeof(buf), "Id=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 51: {
            snprintf(buf, sizeof(buf), "State=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 52: {
            snprintf(buf, sizeof(buf), "Id=%d | SpentFuel=%d | TravelledDistance=(%.1f,%.1f,%.1f)", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10), *(float*)((char*)_this + 0x14));
            return std::string(buf);
        }
        case 53: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 54: {
            snprintf(buf, sizeof(buf), "VehicleId=%d | IsParkingBrakeEnabled=%d", *(int*)((char*)_this + 0x0), *(bool*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 55: {
            snprintf(buf, sizeof(buf), "VehicleId=%d | IsParkingBrakeEnabled=%d", *(int*)((char*)_this + 0x0), *(bool*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 56: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 57: {
            snprintf(buf, sizeof(buf), "Value=%.2f | VehicleId=%d", *(float*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 58: {
            snprintf(buf, sizeof(buf), "RemoteId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 59: {
            snprintf(buf, sizeof(buf), "Model=%d | State=%d | DateEnd=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 60: {
            snprintf(buf, sizeof(buf), "Id=%d | ComponentType=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 61: {
            snprintf(buf, sizeof(buf), "VehicleId=%d | ItemIds=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 62: {
            snprintf(buf, sizeof(buf), "VehicleId=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 63: {
            snprintf(buf, sizeof(buf), "Value=%.2f | VehicleId=%d", *(float*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 64: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d | OpenDoorPointIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 65: {
            snprintf(buf, sizeof(buf), "pedId=%d | vehicleId=%d | seatIndex=%d | openDoorPointIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 66: {
            snprintf(buf, sizeof(buf), "PedId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 67: {
            snprintf(buf, sizeof(buf), "pedId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 68: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d | OpenDoorPointIndex=%d | IsEnter=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12), *(bool*)((char*)_this + 0x14));
            return std::string(buf);
        }
        case 69: {
            snprintf(buf, sizeof(buf), "PedId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 70: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | FromSeatIndex=%d | ToSeatIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 71: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 72: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d | OpenDoorPointIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 73: {
            snprintf(buf, sizeof(buf), "pedId=%d | vehicleId=%d | seatIndex=%d | openDoorPointIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 74: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 75: {
            snprintf(buf, sizeof(buf), "pedId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 76: {
            snprintf(buf, sizeof(buf), "PedId=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 77: {
            snprintf(buf, sizeof(buf), "PedId=%d | VehicleId=%d | SeatIndex=%d | AutoDriverSeat=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x10), *(int*)((char*)_this + 0x12));
            return std::string(buf);
        }
        case 78: {
            snprintf(buf, sizeof(buf), "Id=%d | AvailabilityState=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 79: {
            snprintf(buf, sizeof(buf), "Id=%d | ClassType=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 80: {
            snprintf(buf, sizeof(buf), "Id=%d | Name=%p", *(int*)((char*)_this + 0x0), *(void**)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 81: {
            snprintf(buf, sizeof(buf), "HouseAlias=%d | HouseId=%d | State=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 82: {
            snprintf(buf, sizeof(buf), "StepIndex=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 83: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 84: {
            snprintf(buf, sizeof(buf), "RemoteId=%d | Id=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 85: {
            snprintf(buf, sizeof(buf), "Id=%d | Step=%d | IsRestart=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(bool*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 86: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 87: {
            snprintf(buf, sizeof(buf), "Id=%d | Clothes=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 88: {
            snprintf(buf, sizeof(buf), "Id=%d | FactionType=%d | Rank=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xc));
            return std::string(buf);
        }
        case 89: {
            snprintf(buf, sizeof(buf), "Id=%d | ByPolicemanId=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 90: {
            snprintf(buf, sizeof(buf), "Id=%d | Reason=%d | KillerNickname=%p | SourceName=%p | SourceInventoryItem=%d | LoseConsciousnessDisabledTime=%d | ReviveTime=%d | KillerWeaponLevel=(%.1f,%.1f,%.1f) | KillerClanId=%d | KillerFaction=%d | IsInternshipFaction=%d | KillerClanName=%p", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(void**)((char*)_this + 0x10), *(void**)((char*)_this + 0x18), *(int*)((char*)_this + 0x20), *(int*)((char*)_this + 0x24), *(int*)((char*)_this + 0x28), *(float*)((char*)_this + 0x2c), *(float*)((char*)_this + 0x30), *(float*)((char*)_this + 0x34), *(int*)((char*)_this + 0x30), *(int*)((char*)_this + 0x38), *(bool*)((char*)_this + 0x3a), *(void**)((char*)_this + 0x40));
            return std::string(buf);
        }
        case 91: {
            snprintf(buf, sizeof(buf), "Position=(%.1f,%.1f,%.1f)", *(float*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 92: {
            snprintf(buf, sizeof(buf), "Id=%d | HasOverrideClothes=%d | Clothes=%d", *(int*)((char*)_this + 0x0), *(bool*)((char*)_this + 0x8), *(int*)((char*)_this + 0xa));
            return std::string(buf);
        }
        case 93: {
            snprintf(buf, sizeof(buf), "Id=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 94: {
            snprintf(buf, sizeof(buf), "Id=%d | ConsumeStamina=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 95: {
            snprintf(buf, sizeof(buf), "Id=%d | Place=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(float*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 96: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 97: {
            snprintf(buf, sizeof(buf), "HasValue=%.2f | Value=%.2f", *(float*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4));
            return std::string(buf);
        }
        case 98: {
            snprintf(buf, sizeof(buf), "KeyHorizontal=%d | KeyVertical=%d | KeyVehicleVertical=%d | KeyVehicleHorizontal=%d | StateView=%d | LookAtPosition=(%.1f,%.1f,%.1f) | KeyMenu=%d | KeyCrouch=%d | KeyJump=%d | KeySprint=%d | KeyHandbrake=%d | KeyVehicleGas=%d | KeyVehicleReverse=%d | KeyFire=%d | KeyReload=%d | KeyTarget=%d | KeyAim=%d | KeyMeleeBlock=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x4), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0xc), *(int*)((char*)_this + 0x10), *(float*)((char*)_this + 0x1c), *(float*)((char*)_this + 0x20), *(float*)((char*)_this + 0x24), *(int*)((char*)_this + 0x28), *(int*)((char*)_this + 0x29), *(int*)((char*)_this + 0x2a), *(int*)((char*)_this + 0x2b), *(int*)((char*)_this + 0x2c), *(int*)((char*)_this + 0x2d), *(int*)((char*)_this + 0x2e), *(int*)((char*)_this + 0x2f), *(int*)((char*)_this + 0x30), *(int*)((char*)_this + 0x31), *(int*)((char*)_this + 0x32), *(int*)((char*)_this + 0x33));
            return std::string(buf);
        }
        case 99: {
            snprintf(buf, sizeof(buf), "HasValue=%.2f | Value=%.2f", *(float*)((char*)_this + 0x0), *(float*)((char*)_this + 0x4));
            return std::string(buf);
        }
        case 100: {
            snprintf(buf, sizeof(buf), "InputStateData=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 101: {
            snprintf(buf, sizeof(buf), "PlayerId=%d | InputStateData=%d | Latency=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8), *(int*)((char*)_this + 0x3c));
            return std::string(buf);
        }
        case 102: {
            snprintf(buf, sizeof(buf), "RemoteId=%d | Position=(%.1f,%.1f,%.1f)", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10));
            return std::string(buf);
        }
        case 103: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 104: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 105: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8));
            return std::string(buf);
        }
        case 106: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f | SnapHeight=%d", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8), *(int*)((char*)_this + 0x14));
            return std::string(buf);
        }
        case 107: {
            snprintf(buf, sizeof(buf), "Id=%d | Value=%.2f | ApplyAfterWorldChanged=%d", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8), *(int*)((char*)_this + 0x18));
            return std::string(buf);
        }
        case 108: {
            snprintf(buf, sizeof(buf), "WorldIndex=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 109: {
            snprintf(buf, sizeof(buf), "RemoteId=%d | Position=(%.1f,%.1f,%.1f) | Rotation=(%.1f,%.1f,%.1f) | RemoteTime=%d", *(int*)((char*)_this + 0x0), *(float*)((char*)_this + 0x8), *(float*)((char*)_this + 0xc), *(float*)((char*)_this + 0x10), *(float*)((char*)_this + 0x14), *(float*)((char*)_this + 0x18), *(float*)((char*)_this + 0x1c), *(int*)((char*)_this + 0x28));
            return std::string(buf);
        }
        case 110: {
            snprintf(buf, sizeof(buf), "Class=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 111: {
            snprintf(buf, sizeof(buf), "RecordsCount=%d", *(int*)((char*)_this + 0x0));
            return std::string(buf);
        }
        case 112: {
            snprintf(buf, sizeof(buf), "ElementId=%d | InventoryItem=%d", *(int*)((char*)_this + 0x0), *(int*)((char*)_this + 0x8));
            return std::string(buf);
        }
    }
    return "";
}

void (*orig_IntPlayerResourceChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_IntPlayerResourceChangeDatagram_Read_0(void* _this, void* reader, void* method) {
    if (orig_IntPlayerResourceChangeDatagram_Read) orig_IntPlayerResourceChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "IntPlayerResourceChangeDatagram", format_datagram_fields(_this, 0).c_str());
}

void (*orig_LongPlayerResourceChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_LongPlayerResourceChangeDatagram_Read_1(void* _this, void* reader, void* method) {
    if (orig_LongPlayerResourceChangeDatagram_Read) orig_LongPlayerResourceChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "LongPlayerResourceChangeDatagram", format_datagram_fields(_this, 1).c_str());
}

void (*orig_PlayerResourceLimitChangedDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerResourceLimitChangedDatagram_Read_2(void* _this, void* reader, void* method) {
    if (orig_PlayerResourceLimitChangedDatagram_Read) orig_PlayerResourceLimitChangedDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerResourceLimitChangedDatagram", format_datagram_fields(_this, 2).c_str());
}

void (*orig_PremiumAccountStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PremiumAccountStateDatagram_Read_3(void* _this, void* reader, void* method) {
    if (orig_PremiumAccountStateDatagram_Read) orig_PremiumAccountStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PremiumAccountStateDatagram", format_datagram_fields(_this, 3).c_str());
}

void (*orig_VehicleViewDroppedDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleViewDroppedDatagram_Read_4(void* _this, void* reader, void* method) {
    if (orig_VehicleViewDroppedDatagram_Read) orig_VehicleViewDroppedDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleViewDroppedDatagram", format_datagram_fields(_this, 4).c_str());
}

void (*orig_AccessLevelDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_AccessLevelDatagram_Read_5(void* _this, void* reader, void* method) {
    if (orig_AccessLevelDatagram_Read) orig_AccessLevelDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "AccessLevelDatagram", format_datagram_fields(_this, 5).c_str());
}

void (*orig_DailyTasksPointsDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_DailyTasksPointsDatagram_Read_6(void* _this, void* reader, void* method) {
    if (orig_DailyTasksPointsDatagram_Read) orig_DailyTasksPointsDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "DailyTasksPointsDatagram", format_datagram_fields(_this, 6).c_str());
}

void (*orig_DailyTasksStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_DailyTasksStateDatagram_Read_7(void* _this, void* reader, void* method) {
    if (orig_DailyTasksStateDatagram_Read) orig_DailyTasksStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "DailyTasksStateDatagram", format_datagram_fields(_this, 7).c_str());
}

void (*orig_DestroyElementDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_DestroyElementDatagram_Read_8(void* _this, void* reader, void* method) {
    if (orig_DestroyElementDatagram_Read) orig_DestroyElementDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "DestroyElementDatagram", format_datagram_fields(_this, 8).c_str());
}

void (*orig_DestroyPlayerDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_DestroyPlayerDatagram_Read_9(void* _this, void* reader, void* method) {
    if (orig_DestroyPlayerDatagram_Read) orig_DestroyPlayerDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "DestroyPlayerDatagram", format_datagram_fields(_this, 9).c_str());
}

void (*orig_ElementControlsElementDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementControlsElementDatagram_Read_10(void* _this, void* reader, void* method) {
    if (orig_ElementControlsElementDatagram_Read) orig_ElementControlsElementDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementControlsElementDatagram", format_datagram_fields(_this, 10).c_str());
}

void (*orig_ElementStartSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementStartSyncDatagram_Read_11(void* _this, void* reader, void* method) {
    if (orig_ElementStartSyncDatagram_Read) orig_ElementStartSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementStartSyncDatagram", format_datagram_fields(_this, 11).c_str());
}

void (*orig_ElementStopSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementStopSyncDatagram_Read_12(void* _this, void* reader, void* method) {
    if (orig_ElementStopSyncDatagram_Read) orig_ElementStopSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementStopSyncDatagram", format_datagram_fields(_this, 12).c_str());
}

void (*orig_CheatVehicleSpawnResponseDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_CheatVehicleSpawnResponseDatagram_Read_13(void* _this, void* reader, void* method) {
    if (orig_CheatVehicleSpawnResponseDatagram_Read) orig_CheatVehicleSpawnResponseDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "CheatVehicleSpawnResponseDatagram", format_datagram_fields(_this, 13).c_str());
}

void (*orig_CreateElementDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_CreateElementDatagram_Read_14(void* _this, void* reader, void* method) {
    if (orig_CreateElementDatagram_Read) orig_CreateElementDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "CreateElementDatagram", format_datagram_fields(_this, 14).c_str());
}

void (*orig_ElementSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementSyncDatagram_Read_15(void* _this, void* reader, void* method) {
    if (orig_ElementSyncDatagram_Read) orig_ElementSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementSyncDatagram", format_datagram_fields(_this, 15).c_str());
}

void (*orig_IconDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_IconDatagram_Read_16(void* _this, void* reader, void* method) {
    if (orig_IconDatagram_Read) orig_IconDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "IconDatagram", format_datagram_fields(_this, 16).c_str());
}

void (*orig_LatencyRequestDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_LatencyRequestDatagram_Read_17(void* _this, void* reader, void* method) {
    if (orig_LatencyRequestDatagram_Read) orig_LatencyRequestDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "LatencyRequestDatagram", format_datagram_fields(_this, 17).c_str());
}

void (*orig_LatencyResponseDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_LatencyResponseDatagram_Read_18(void* _this, void* reader, void* method) {
    if (orig_LatencyResponseDatagram_Read) orig_LatencyResponseDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "LatencyResponseDatagram", format_datagram_fields(_this, 18).c_str());
}

void (*orig_PlayerControlsElementDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerControlsElementDatagram_Read_19(void* _this, void* reader, void* method) {
    if (orig_PlayerControlsElementDatagram_Read) orig_PlayerControlsElementDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerControlsElementDatagram", format_datagram_fields(_this, 19).c_str());
}

void (*orig_PlayerCreateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerCreateDatagram_Read_20(void* _this, void* reader, void* method) {
    if (orig_PlayerCreateDatagram_Read) orig_PlayerCreateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerCreateDatagram", format_datagram_fields(_this, 20).c_str());
}

void (*orig_PlayerDespawnAnyVehicleDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerDespawnAnyVehicleDatagram_Read_21(void* _this, void* reader, void* method) {
    if (orig_PlayerDespawnAnyVehicleDatagram_Read) orig_PlayerDespawnAnyVehicleDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerDespawnAnyVehicleDatagram", format_datagram_fields(_this, 21).c_str());
}

void (*orig_PlayerJailEndTimeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerJailEndTimeDatagram_Read_22(void* _this, void* reader, void* method) {
    if (orig_PlayerJailEndTimeDatagram_Read) orig_PlayerJailEndTimeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerJailEndTimeDatagram", format_datagram_fields(_this, 22).c_str());
}

void (*orig_PlayerMoneyTransfersInfoDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerMoneyTransfersInfoDatagram_Read_23(void* _this, void* reader, void* method) {
    if (orig_PlayerMoneyTransfersInfoDatagram_Read) orig_PlayerMoneyTransfersInfoDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerMoneyTransfersInfoDatagram", format_datagram_fields(_this, 23).c_str());
}

void (*orig_PlayerParkingZoneDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerParkingZoneDatagram_Read_24(void* _this, void* reader, void* method) {
    if (orig_PlayerParkingZoneDatagram_Read) orig_PlayerParkingZoneDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerParkingZoneDatagram", format_datagram_fields(_this, 24).c_str());
}

void (*orig_PlayerTechVehiclePassportDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerTechVehiclePassportDatagram_Read_25(void* _this, void* reader, void* method) {
    if (orig_PlayerTechVehiclePassportDatagram_Read) orig_PlayerTechVehiclePassportDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerTechVehiclePassportDatagram", format_datagram_fields(_this, 25).c_str());
}

void (*orig_PlayerTuningSessionDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerTuningSessionDatagram_Read_26(void* _this, void* reader, void* method) {
    if (orig_PlayerTuningSessionDatagram_Read) orig_PlayerTuningSessionDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerTuningSessionDatagram", format_datagram_fields(_this, 26).c_str());
}

void (*orig_PlayerVehiclesKeysDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerVehiclesKeysDatagram_Read_27(void* _this, void* reader, void* method) {
    if (orig_PlayerVehiclesKeysDatagram_Read) orig_PlayerVehiclesKeysDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerVehiclesKeysDatagram", format_datagram_fields(_this, 27).c_str());
}

void (*orig_PlayerVoiceMuteDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerVoiceMuteDatagram_Read_28(void* _this, void* reader, void* method) {
    if (orig_PlayerVoiceMuteDatagram_Read) orig_PlayerVoiceMuteDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerVoiceMuteDatagram", format_datagram_fields(_this, 28).c_str());
}

void (*orig_RadialTutorDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_RadialTutorDatagram_Read_29(void* _this, void* reader, void* method) {
    if (orig_RadialTutorDatagram_Read) orig_RadialTutorDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "RadialTutorDatagram", format_datagram_fields(_this, 29).c_str());
}

void (*orig_SetElementDimensionDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_SetElementDimensionDatagram_Read_30(void* _this, void* reader, void* method) {
    if (orig_SetElementDimensionDatagram_Read) orig_SetElementDimensionDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "SetElementDimensionDatagram", format_datagram_fields(_this, 30).c_str());
}

void (*orig_SetPlayerDimensionDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_SetPlayerDimensionDatagram_Read_31(void* _this, void* reader, void* method) {
    if (orig_SetPlayerDimensionDatagram_Read) orig_SetPlayerDimensionDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "SetPlayerDimensionDatagram", format_datagram_fields(_this, 31).c_str());
}

void (*orig_SyncPedRagdollDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_SyncPedRagdollDatagram_Read_32(void* _this, void* reader, void* method) {
    if (orig_SyncPedRagdollDatagram_Read) orig_SyncPedRagdollDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "SyncPedRagdollDatagram", format_datagram_fields(_this, 32).c_str());
}

void (*orig_TimeCycleSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_TimeCycleSyncDatagram_Read_33(void* _this, void* reader, void* method) {
    if (orig_TimeCycleSyncDatagram_Read) orig_TimeCycleSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "TimeCycleSyncDatagram", format_datagram_fields(_this, 33).c_str());
}

void (*orig_TwoIntPlayerResourceChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_TwoIntPlayerResourceChangeDatagram_Read_34(void* _this, void* reader, void* method) {
    if (orig_TwoIntPlayerResourceChangeDatagram_Read) orig_TwoIntPlayerResourceChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "TwoIntPlayerResourceChangeDatagram", format_datagram_fields(_this, 34).c_str());
}

void (*orig_UnderwaterDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_UnderwaterDatagram_Read_35(void* _this, void* reader, void* method) {
    if (orig_UnderwaterDatagram_Read) orig_UnderwaterDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "UnderwaterDatagram", format_datagram_fields(_this, 35).c_str());
}

void (*orig_VehicleChangedDoorStateSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleChangedDoorStateSyncDatagram_Read_36(void* _this, void* reader, void* method) {
    if (orig_VehicleChangedDoorStateSyncDatagram_Read) orig_VehicleChangedDoorStateSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleChangedDoorStateSyncDatagram", format_datagram_fields(_this, 36).c_str());
}

void (*orig_VehicleHeadLightsStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleHeadLightsStateDatagram_Read_37(void* _this, void* reader, void* method) {
    if (orig_VehicleHeadLightsStateDatagram_Read) orig_VehicleHeadLightsStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleHeadLightsStateDatagram", format_datagram_fields(_this, 37).c_str());
}

void (*orig_VehicleTurnLightsStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleTurnLightsStateDatagram_Read_38(void* _this, void* reader, void* method) {
    if (orig_VehicleTurnLightsStateDatagram_Read) orig_VehicleTurnLightsStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleTurnLightsStateDatagram", format_datagram_fields(_this, 38).c_str());
}

void (*orig_ViolationCountDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ViolationCountDatagram_Read_39(void* _this, void* reader, void* method) {
    if (orig_ViolationCountDatagram_Read) orig_ViolationCountDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ViolationCountDatagram", format_datagram_fields(_this, 39).c_str());
}

void (*orig_WeaponShotFromClientDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_WeaponShotFromClientDatagram_Read_40(void* _this, void* reader, void* method) {
    if (orig_WeaponShotFromClientDatagram_Read) orig_WeaponShotFromClientDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "WeaponShotFromClientDatagram", format_datagram_fields(_this, 40).c_str());
}

void (*orig_WeaponShotFromServerDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_WeaponShotFromServerDatagram_Read_41(void* _this, void* reader, void* method) {
    if (orig_WeaponShotFromServerDatagram_Read) orig_WeaponShotFromServerDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "WeaponShotFromServerDatagram", format_datagram_fields(_this, 41).c_str());
}

void (*orig_VehicleChangeDoorStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleChangeDoorStateDatagram_Read_42(void* _this, void* reader, void* method) {
    if (orig_VehicleChangeDoorStateDatagram_Read) orig_VehicleChangeDoorStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleChangeDoorStateDatagram", format_datagram_fields(_this, 42).c_str());
}

void (*orig_VehicleChangeFuelDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleChangeFuelDatagram_Read_43(void* _this, void* reader, void* method) {
    if (orig_VehicleChangeFuelDatagram_Read) orig_VehicleChangeFuelDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleChangeFuelDatagram", format_datagram_fields(_this, 43).c_str());
}

void (*orig_VehicleChangeLockStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleChangeLockStateDatagram_Read_44(void* _this, void* reader, void* method) {
    if (orig_VehicleChangeLockStateDatagram_Read) orig_VehicleChangeLockStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleChangeLockStateDatagram", format_datagram_fields(_this, 44).c_str());
}

void (*orig_VehicleChangeMileageDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleChangeMileageDatagram_Read_45(void* _this, void* reader, void* method) {
    if (orig_VehicleChangeMileageDatagram_Read) orig_VehicleChangeMileageDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleChangeMileageDatagram", format_datagram_fields(_this, 45).c_str());
}

void (*orig_VehicleColorDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleColorDatagram_Read_46(void* _this, void* reader, void* method) {
    if (orig_VehicleColorDatagram_Read) orig_VehicleColorDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleColorDatagram", format_datagram_fields(_this, 46).c_str());
}

void (*orig_VehicleDrivingSchoolDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleDrivingSchoolDatagram_Read_47(void* _this, void* reader, void* method) {
    if (orig_VehicleDrivingSchoolDatagram_Read) orig_VehicleDrivingSchoolDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleDrivingSchoolDatagram", format_datagram_fields(_this, 47).c_str());
}

void (*orig_VehicleDroppedDownDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleDroppedDownDatagram_Read_48(void* _this, void* reader, void* method) {
    if (orig_VehicleDroppedDownDatagram_Read) orig_VehicleDroppedDownDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleDroppedDownDatagram", format_datagram_fields(_this, 48).c_str());
}

void (*orig_VehicleEmergencyLightsStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleEmergencyLightsStateDatagram_Read_49(void* _this, void* reader, void* method) {
    if (orig_VehicleEmergencyLightsStateDatagram_Read) orig_VehicleEmergencyLightsStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleEmergencyLightsStateDatagram", format_datagram_fields(_this, 49).c_str());
}

void (*orig_VehicleEngineRunningDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleEngineRunningDatagram_Read_50(void* _this, void* reader, void* method) {
    if (orig_VehicleEngineRunningDatagram_Read) orig_VehicleEngineRunningDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleEngineRunningDatagram", format_datagram_fields(_this, 50).c_str());
}

void (*orig_VehicleHornDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleHornDatagram_Read_51(void* _this, void* reader, void* method) {
    if (orig_VehicleHornDatagram_Read) orig_VehicleHornDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleHornDatagram", format_datagram_fields(_this, 51).c_str());
}

void (*orig_VehicleOdometerUpdateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleOdometerUpdateDatagram_Read_52(void* _this, void* reader, void* method) {
    if (orig_VehicleOdometerUpdateDatagram_Read) orig_VehicleOdometerUpdateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleOdometerUpdateDatagram", format_datagram_fields(_this, 52).c_str());
}

void (*orig_VehicleOwnDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleOwnDatagram_Read_53(void* _this, void* reader, void* method) {
    if (orig_VehicleOwnDatagram_Read) orig_VehicleOwnDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleOwnDatagram", format_datagram_fields(_this, 53).c_str());
}

void (*orig_VehicleParkingBrakeChangedCtsDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleParkingBrakeChangedCtsDatagram_Read_54(void* _this, void* reader, void* method) {
    if (orig_VehicleParkingBrakeChangedCtsDatagram_Read) orig_VehicleParkingBrakeChangedCtsDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleParkingBrakeChangedCtsDatagram", format_datagram_fields(_this, 54).c_str());
}

void (*orig_VehicleParkingBrakeChangedStcDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleParkingBrakeChangedStcDatagram_Read_55(void* _this, void* reader, void* method) {
    if (orig_VehicleParkingBrakeChangedStcDatagram_Read) orig_VehicleParkingBrakeChangedStcDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleParkingBrakeChangedStcDatagram", format_datagram_fields(_this, 55).c_str());
}

void (*orig_VehiclePositionTeleportDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehiclePositionTeleportDatagram_Read_56(void* _this, void* reader, void* method) {
    if (orig_VehiclePositionTeleportDatagram_Read) orig_VehiclePositionTeleportDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehiclePositionTeleportDatagram", format_datagram_fields(_this, 56).c_str());
}

void (*orig_VehicleReputationDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleReputationDatagram_Read_57(void* _this, void* reader, void* method) {
    if (orig_VehicleReputationDatagram_Read) orig_VehicleReputationDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleReputationDatagram", format_datagram_fields(_this, 57).c_str());
}

void (*orig_VehicleTemporaryDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleTemporaryDatagram_Read_58(void* _this, void* reader, void* method) {
    if (orig_VehicleTemporaryDatagram_Read) orig_VehicleTemporaryDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleTemporaryDatagram", format_datagram_fields(_this, 58).c_str());
}

void (*orig_VehicleTemporaryNetworkModel_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleTemporaryNetworkModel_Read_59(void* _this, void* reader, void* method) {
    if (orig_VehicleTemporaryNetworkModel_Read) orig_VehicleTemporaryNetworkModel_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleTemporaryNetworkModel", format_datagram_fields(_this, 59).c_str());
}

void (*orig_VehicleTuningComponentChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleTuningComponentChangeDatagram_Read_60(void* _this, void* reader, void* method) {
    if (orig_VehicleTuningComponentChangeDatagram_Read) orig_VehicleTuningComponentChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleTuningComponentChangeDatagram", format_datagram_fields(_this, 60).c_str());
}

void (*orig_VehicleTuningItemsDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleTuningItemsDatagram_Read_61(void* _this, void* reader, void* method) {
    if (orig_VehicleTuningItemsDatagram_Read) orig_VehicleTuningItemsDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleTuningItemsDatagram", format_datagram_fields(_this, 61).c_str());
}

void (*orig_VehicleViewHornDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleViewHornDatagram_Read_62(void* _this, void* reader, void* method) {
    if (orig_VehicleViewHornDatagram_Read) orig_VehicleViewHornDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleViewHornDatagram", format_datagram_fields(_this, 62).c_str());
}

void (*orig_VehicleVinDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_VehicleVinDatagram_Read_63(void* _this, void* reader, void* method) {
    if (orig_VehicleVinDatagram_Read) orig_VehicleVinDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "VehicleVinDatagram", format_datagram_fields(_this, 63).c_str());
}

void (*orig_PedEnterToVehicleCTSDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedEnterToVehicleCTSDatagram_Read_64(void* _this, void* reader, void* method) {
    if (orig_PedEnterToVehicleCTSDatagram_Read) orig_PedEnterToVehicleCTSDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedEnterToVehicleCTSDatagram", format_datagram_fields(_this, 64).c_str());
}

void (*orig_PedEnterToVehicleSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedEnterToVehicleSTCDatagram_Read_65(void* _this, void* reader, void* method) {
    if (orig_PedEnterToVehicleSTCDatagram_Read) orig_PedEnterToVehicleSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedEnterToVehicleSTCDatagram", format_datagram_fields(_this, 65).c_str());
}

void (*orig_PedExitFromVehicleCTSDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedExitFromVehicleCTSDatagram_Read_66(void* _this, void* reader, void* method) {
    if (orig_PedExitFromVehicleCTSDatagram_Read) orig_PedExitFromVehicleCTSDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedExitFromVehicleCTSDatagram", format_datagram_fields(_this, 66).c_str());
}

void (*orig_PedExitFromVehicleSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedExitFromVehicleSTCDatagram_Read_67(void* _this, void* reader, void* method) {
    if (orig_PedExitFromVehicleSTCDatagram_Read) orig_PedExitFromVehicleSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedExitFromVehicleSTCDatagram", format_datagram_fields(_this, 67).c_str());
}

void (*orig_PedOpenVehicleDoorDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedOpenVehicleDoorDatagram_Read_68(void* _this, void* reader, void* method) {
    if (orig_PedOpenVehicleDoorDatagram_Read) orig_PedOpenVehicleDoorDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedOpenVehicleDoorDatagram", format_datagram_fields(_this, 68).c_str());
}

void (*orig_PedSwitchRejectVehicleSeatSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedSwitchRejectVehicleSeatSTCDatagram_Read_69(void* _this, void* reader, void* method) {
    if (orig_PedSwitchRejectVehicleSeatSTCDatagram_Read) orig_PedSwitchRejectVehicleSeatSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedSwitchRejectVehicleSeatSTCDatagram", format_datagram_fields(_this, 69).c_str());
}

void (*orig_PedSwitchVehicleSeatCTSDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedSwitchVehicleSeatCTSDatagram_Read_70(void* _this, void* reader, void* method) {
    if (orig_PedSwitchVehicleSeatCTSDatagram_Read) orig_PedSwitchVehicleSeatCTSDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedSwitchVehicleSeatCTSDatagram", format_datagram_fields(_this, 70).c_str());
}

void (*orig_PedSwitchVehicleSeatSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedSwitchVehicleSeatSTCDatagram_Read_71(void* _this, void* reader, void* method) {
    if (orig_PedSwitchVehicleSeatSTCDatagram_Read) orig_PedSwitchVehicleSeatSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedSwitchVehicleSeatSTCDatagram", format_datagram_fields(_this, 71).c_str());
}

void (*orig_PedTryEnterToVehicleCTSDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedTryEnterToVehicleCTSDatagram_Read_72(void* _this, void* reader, void* method) {
    if (orig_PedTryEnterToVehicleCTSDatagram_Read) orig_PedTryEnterToVehicleCTSDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedTryEnterToVehicleCTSDatagram", format_datagram_fields(_this, 72).c_str());
}

void (*orig_PedTryEnterToVehicleSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedTryEnterToVehicleSTCDatagram_Read_73(void* _this, void* reader, void* method) {
    if (orig_PedTryEnterToVehicleSTCDatagram_Read) orig_PedTryEnterToVehicleSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedTryEnterToVehicleSTCDatagram", format_datagram_fields(_this, 73).c_str());
}

void (*orig_PedTryExitFromVehicleCTSDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedTryExitFromVehicleCTSDatagram_Read_74(void* _this, void* reader, void* method) {
    if (orig_PedTryExitFromVehicleCTSDatagram_Read) orig_PedTryExitFromVehicleCTSDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedTryExitFromVehicleCTSDatagram", format_datagram_fields(_this, 74).c_str());
}

void (*orig_PedTryExitFromVehicleSTCDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedTryExitFromVehicleSTCDatagram_Read_75(void* _this, void* reader, void* method) {
    if (orig_PedTryExitFromVehicleSTCDatagram_Read) orig_PedTryExitFromVehicleSTCDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedTryExitFromVehicleSTCDatagram", format_datagram_fields(_this, 75).c_str());
}

void (*orig_RemovePedSeatDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_RemovePedSeatDatagram_Read_76(void* _this, void* reader, void* method) {
    if (orig_RemovePedSeatDatagram_Read) orig_RemovePedSeatDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "RemovePedSeatDatagram", format_datagram_fields(_this, 76).c_str());
}

void (*orig_ReplacePedSeatDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ReplacePedSeatDatagram_Read_77(void* _this, void* reader, void* method) {
    if (orig_ReplacePedSeatDatagram_Read) orig_ReplacePedSeatDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ReplacePedSeatDatagram", format_datagram_fields(_this, 77).c_str());
}

void (*orig_HouseAvailabilityStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_HouseAvailabilityStateDatagram_Read_78(void* _this, void* reader, void* method) {
    if (orig_HouseAvailabilityStateDatagram_Read) orig_HouseAvailabilityStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "HouseAvailabilityStateDatagram", format_datagram_fields(_this, 78).c_str());
}

void (*orig_HouseClassTypeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_HouseClassTypeDatagram_Read_79(void* _this, void* reader, void* method) {
    if (orig_HouseClassTypeDatagram_Read) orig_HouseClassTypeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "HouseClassTypeDatagram", format_datagram_fields(_this, 79).c_str());
}

void (*orig_HouseNameDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_HouseNameDatagram_Read_80(void* _this, void* reader, void* method) {
    if (orig_HouseNameDatagram_Read) orig_HouseNameDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "HouseNameDatagram", format_datagram_fields(_this, 80).c_str());
}

void (*orig_HouseParkingVisible_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_HouseParkingVisible_Read_81(void* _this, void* reader, void* method) {
    if (orig_HouseParkingVisible_Read) orig_HouseParkingVisible_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "HouseParkingVisible", format_datagram_fields(_this, 81).c_str());
}

void (*orig_RequestCompleteQuestStepDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_RequestCompleteQuestStepDatagram_Read_82(void* _this, void* reader, void* method) {
    if (orig_RequestCompleteQuestStepDatagram_Read) orig_RequestCompleteQuestStepDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "RequestCompleteQuestStepDatagram", format_datagram_fields(_this, 82).c_str());
}

void (*orig_RequestStartQuestDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_RequestStartQuestDatagram_Read_83(void* _this, void* reader, void* method) {
    if (orig_RequestStartQuestDatagram_Read) orig_RequestStartQuestDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "RequestStartQuestDatagram", format_datagram_fields(_this, 83).c_str());
}

void (*orig_SetQuestElementDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_SetQuestElementDatagram_Read_84(void* _this, void* reader, void* method) {
    if (orig_SetQuestElementDatagram_Read) orig_SetQuestElementDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "SetQuestElementDatagram", format_datagram_fields(_this, 84).c_str());
}

void (*orig_UpdateQuestStepDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_UpdateQuestStepDatagram_Read_85(void* _this, void* reader, void* method) {
    if (orig_UpdateQuestStepDatagram_Read) orig_UpdateQuestStepDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "UpdateQuestStepDatagram", format_datagram_fields(_this, 85).c_str());
}

void (*orig_PedBlockRotationDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedBlockRotationDatagram_Read_86(void* _this, void* reader, void* method) {
    if (orig_PedBlockRotationDatagram_Read) orig_PedBlockRotationDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedBlockRotationDatagram", format_datagram_fields(_this, 86).c_str());
}

void (*orig_PedClothesChangedDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedClothesChangedDatagram_Read_87(void* _this, void* reader, void* method) {
    if (orig_PedClothesChangedDatagram_Read) orig_PedClothesChangedDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedClothesChangedDatagram", format_datagram_fields(_this, 87).c_str());
}

void (*orig_PedClothesVariantsChangedDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedClothesVariantsChangedDatagram_Read_88(void* _this, void* reader, void* method) {
    if (orig_PedClothesVariantsChangedDatagram_Read) orig_PedClothesVariantsChangedDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedClothesVariantsChangedDatagram", format_datagram_fields(_this, 88).c_str());
}

void (*orig_PedCuffDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedCuffDatagram_Read_89(void* _this, void* reader, void* method) {
    if (orig_PedCuffDatagram_Read) orig_PedCuffDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedCuffDatagram", format_datagram_fields(_this, 89).c_str());
}

void (*orig_PedDeadDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedDeadDatagram_Read_90(void* _this, void* reader, void* method) {
    if (orig_PedDeadDatagram_Read) orig_PedDeadDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedDeadDatagram", format_datagram_fields(_this, 90).c_str());
}

void (*orig_PedInteriorPositionDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedInteriorPositionDatagram_Read_91(void* _this, void* reader, void* method) {
    if (orig_PedInteriorPositionDatagram_Read) orig_PedInteriorPositionDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedInteriorPositionDatagram", format_datagram_fields(_this, 91).c_str());
}

void (*orig_PedOverrideClothesChangedDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedOverrideClothesChangedDatagram_Read_92(void* _this, void* reader, void* method) {
    if (orig_PedOverrideClothesChangedDatagram_Read) orig_PedOverrideClothesChangedDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedOverrideClothesChangedDatagram", format_datagram_fields(_this, 92).c_str());
}

void (*orig_PedReviveDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedReviveDatagram_Read_93(void* _this, void* reader, void* method) {
    if (orig_PedReviveDatagram_Read) orig_PedReviveDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedReviveDatagram", format_datagram_fields(_this, 93).c_str());
}

void (*orig_PedStaminaStateDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PedStaminaStateDatagram_Read_94(void* _this, void* reader, void* method) {
    if (orig_PedStaminaStateDatagram_Read) orig_PedStaminaStateDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PedStaminaStateDatagram", format_datagram_fields(_this, 94).c_str());
}

void (*orig_NicknameAdditionalDataDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_NicknameAdditionalDataDatagram_Read_95(void* _this, void* reader, void* method) {
    if (orig_NicknameAdditionalDataDatagram_Read) orig_NicknameAdditionalDataDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "NicknameAdditionalDataDatagram", format_datagram_fields(_this, 95).c_str());
}

void (*orig_NicknameColorDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_NicknameColorDatagram_Read_96(void* _this, void* reader, void* method) {
    if (orig_NicknameColorDatagram_Read) orig_NicknameColorDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "NicknameColorDatagram", format_datagram_fields(_this, 96).c_str());
}

void (*orig_InputFloat_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_InputFloat_Read_97(void* _this, void* reader, void* method) {
    if (orig_InputFloat_Read) orig_InputFloat_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "InputFloat", format_datagram_fields(_this, 97).c_str());
}

void (*orig_InputStateData_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_InputStateData_Read_98(void* _this, void* reader, void* method) {
    if (orig_InputStateData_Read) orig_InputStateData_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "InputStateData", format_datagram_fields(_this, 98).c_str());
}

void (*orig_InputVector3_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_InputVector3_Read_99(void* _this, void* reader, void* method) {
    if (orig_InputVector3_Read) orig_InputVector3_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "InputVector3", format_datagram_fields(_this, 99).c_str());
}

void (*orig_PlayerInputStateInfoDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerInputStateInfoDatagram_Read_100(void* _this, void* reader, void* method) {
    if (orig_PlayerInputStateInfoDatagram_Read) orig_PlayerInputStateInfoDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerInputStateInfoDatagram", format_datagram_fields(_this, 100).c_str());
}

void (*orig_UpdatePlayerInputStateInfoDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_UpdatePlayerInputStateInfoDatagram_Read_101(void* _this, void* reader, void* method) {
    if (orig_UpdatePlayerInputStateInfoDatagram_Read) orig_UpdatePlayerInputStateInfoDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "UpdatePlayerInputStateInfoDatagram", format_datagram_fields(_this, 101).c_str());
}

void (*orig_AttachedDisplacementSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_AttachedDisplacementSyncDatagram_Read_102(void* _this, void* reader, void* method) {
    if (orig_AttachedDisplacementSyncDatagram_Read) orig_AttachedDisplacementSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "AttachedDisplacementSyncDatagram", format_datagram_fields(_this, 102).c_str());
}

void (*orig_ElementFreezeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementFreezeDatagram_Read_103(void* _this, void* reader, void* method) {
    if (orig_ElementFreezeDatagram_Read) orig_ElementFreezeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementFreezeDatagram", format_datagram_fields(_this, 103).c_str());
}

void (*orig_ElementModelDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementModelDatagram_Read_104(void* _this, void* reader, void* method) {
    if (orig_ElementModelDatagram_Read) orig_ElementModelDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementModelDatagram", format_datagram_fields(_this, 104).c_str());
}

void (*orig_ElementPositionDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementPositionDatagram_Read_105(void* _this, void* reader, void* method) {
    if (orig_ElementPositionDatagram_Read) orig_ElementPositionDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementPositionDatagram", format_datagram_fields(_this, 105).c_str());
}

void (*orig_ElementPositionWithSnapDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementPositionWithSnapDatagram_Read_106(void* _this, void* reader, void* method) {
    if (orig_ElementPositionWithSnapDatagram_Read) orig_ElementPositionWithSnapDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementPositionWithSnapDatagram", format_datagram_fields(_this, 106).c_str());
}

void (*orig_ElementRotationDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementRotationDatagram_Read_107(void* _this, void* reader, void* method) {
    if (orig_ElementRotationDatagram_Read) orig_ElementRotationDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementRotationDatagram", format_datagram_fields(_this, 107).c_str());
}

void (*orig_ElementWorldDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ElementWorldDatagram_Read_108(void* _this, void* reader, void* method) {
    if (orig_ElementWorldDatagram_Read) orig_ElementWorldDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ElementWorldDatagram", format_datagram_fields(_this, 108).c_str());
}

void (*orig_ObjectDisplacementSyncDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_ObjectDisplacementSyncDatagram_Read_109(void* _this, void* reader, void* method) {
    if (orig_ObjectDisplacementSyncDatagram_Read) orig_ObjectDisplacementSyncDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "ObjectDisplacementSyncDatagram", format_datagram_fields(_this, 109).c_str());
}

void (*orig_PlayerDriverLicenseChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerDriverLicenseChangeDatagram_Read_110(void* _this, void* reader, void* method) {
    if (orig_PlayerDriverLicenseChangeDatagram_Read) orig_PlayerDriverLicenseChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerDriverLicenseChangeDatagram", format_datagram_fields(_this, 110).c_str());
}

void (*orig_PlayerEmploymentBookChangeDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_PlayerEmploymentBookChangeDatagram_Read_111(void* _this, void* reader, void* method) {
    if (orig_PlayerEmploymentBookChangeDatagram_Read) orig_PlayerEmploymentBookChangeDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "PlayerEmploymentBookChangeDatagram", format_datagram_fields(_this, 111).c_str());
}

void (*orig_WeaponTizerEffectDatagram_Read)(void*, void*, void*) = nullptr;
extern "C" void hook_WeaponTizerEffectDatagram_Read_112(void* _this, void* reader, void* method) {
    if (orig_WeaponTizerEffectDatagram_Read) orig_WeaponTizerEffectDatagram_Read(_this, reader, method);
    if (!_this) return;
    AB_LOGI("DATAGRAM [Auto] %s: %s", "WeaponTizerEffectDatagram", format_datagram_fields(_this, 112).c_str());
}

void Aimbot_InstallAutoSniffers(uintptr_t libBase) {
    AB_LOGI("Installing %d Auto-Sniffers...", 113);
    DobbyHook((void*)(libBase + 0x2FBBD50), (void*)hook_IntPlayerResourceChangeDatagram_Read_0, (void**)&orig_IntPlayerResourceChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2FBBE54), (void*)hook_LongPlayerResourceChangeDatagram_Read_1, (void**)&orig_LongPlayerResourceChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2FBBF8C), (void*)hook_PlayerResourceLimitChangedDatagram_Read_2, (void**)&orig_PlayerResourceLimitChangedDatagram_Read);
    DobbyHook((void*)(libBase + 0x2FBC0E4), (void*)hook_PremiumAccountStateDatagram_Read_3, (void**)&orig_PremiumAccountStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2FBC268), (void*)hook_VehicleViewDroppedDatagram_Read_4, (void**)&orig_VehicleViewDroppedDatagram_Read);
    DobbyHook((void*)(libBase + 0x25610B0), (void*)hook_AccessLevelDatagram_Read_5, (void**)&orig_AccessLevelDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561230), (void*)hook_DailyTasksPointsDatagram_Read_6, (void**)&orig_DailyTasksPointsDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561498), (void*)hook_DailyTasksStateDatagram_Read_7, (void**)&orig_DailyTasksStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561580), (void*)hook_DestroyElementDatagram_Read_8, (void**)&orig_DestroyElementDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561654), (void*)hook_DestroyPlayerDatagram_Read_9, (void**)&orig_DestroyPlayerDatagram_Read);
    DobbyHook((void*)(libBase + 0x256173C), (void*)hook_ElementControlsElementDatagram_Read_10, (void**)&orig_ElementControlsElementDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561898), (void*)hook_ElementStartSyncDatagram_Read_11, (void**)&orig_ElementStartSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x256196C), (void*)hook_ElementStopSyncDatagram_Read_12, (void**)&orig_ElementStopSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561A40), (void*)hook_CheatVehicleSpawnResponseDatagram_Read_13, (void**)&orig_CheatVehicleSpawnResponseDatagram_Read);
    DobbyHook((void*)(libBase + 0x2561D3C), (void*)hook_CreateElementDatagram_Read_14, (void**)&orig_CreateElementDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562198), (void*)hook_ElementSyncDatagram_Read_15, (void**)&orig_ElementSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562304), (void*)hook_IconDatagram_Read_16, (void**)&orig_IconDatagram_Read);
    DobbyHook((void*)(libBase + 0x25625F8), (void*)hook_LatencyRequestDatagram_Read_17, (void**)&orig_LatencyRequestDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562648), (void*)hook_LatencyResponseDatagram_Read_18, (void**)&orig_LatencyResponseDatagram_Read);
    DobbyHook((void*)(libBase + 0x25626EC), (void*)hook_PlayerControlsElementDatagram_Read_19, (void**)&orig_PlayerControlsElementDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562884), (void*)hook_PlayerCreateDatagram_Read_20, (void**)&orig_PlayerCreateDatagram_Read);
    DobbyHook((void*)(libBase + 0x25629D0), (void*)hook_PlayerDespawnAnyVehicleDatagram_Read_21, (void**)&orig_PlayerDespawnAnyVehicleDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562DD8), (void*)hook_PlayerJailEndTimeDatagram_Read_22, (void**)&orig_PlayerJailEndTimeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562ED0), (void*)hook_PlayerMoneyTransfersInfoDatagram_Read_23, (void**)&orig_PlayerMoneyTransfersInfoDatagram_Read);
    DobbyHook((void*)(libBase + 0x2562FA4), (void*)hook_PlayerParkingZoneDatagram_Read_24, (void**)&orig_PlayerParkingZoneDatagram_Read);
    DobbyHook((void*)(libBase + 0x25630CC), (void*)hook_PlayerTechVehiclePassportDatagram_Read_25, (void**)&orig_PlayerTechVehiclePassportDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563250), (void*)hook_PlayerTuningSessionDatagram_Read_26, (void**)&orig_PlayerTuningSessionDatagram_Read);
    DobbyHook((void*)(libBase + 0x256354C), (void*)hook_PlayerVehiclesKeysDatagram_Read_27, (void**)&orig_PlayerVehiclesKeysDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563700), (void*)hook_PlayerVoiceMuteDatagram_Read_28, (void**)&orig_PlayerVoiceMuteDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563828), (void*)hook_RadialTutorDatagram_Read_29, (void**)&orig_RadialTutorDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563990), (void*)hook_SetElementDimensionDatagram_Read_30, (void**)&orig_SetElementDimensionDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563ABC), (void*)hook_SetPlayerDimensionDatagram_Read_31, (void**)&orig_SetPlayerDimensionDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563C5C), (void*)hook_SyncPedRagdollDatagram_Read_32, (void**)&orig_SyncPedRagdollDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563E34), (void*)hook_TimeCycleSyncDatagram_Read_33, (void**)&orig_TimeCycleSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x2563F90), (void*)hook_TwoIntPlayerResourceChangeDatagram_Read_34, (void**)&orig_TwoIntPlayerResourceChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564148), (void*)hook_UnderwaterDatagram_Read_35, (void**)&orig_UnderwaterDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564324), (void*)hook_VehicleChangedDoorStateSyncDatagram_Read_36, (void**)&orig_VehicleChangedDoorStateSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x256447C), (void*)hook_VehicleHeadLightsStateDatagram_Read_37, (void**)&orig_VehicleHeadLightsStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x25645FC), (void*)hook_VehicleTurnLightsStateDatagram_Read_38, (void**)&orig_VehicleTurnLightsStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x256474C), (void*)hook_ViolationCountDatagram_Read_39, (void**)&orig_ViolationCountDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564878), (void*)hook_WeaponShotFromClientDatagram_Read_40, (void**)&orig_WeaponShotFromClientDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564ADC), (void*)hook_WeaponShotFromServerDatagram_Read_41, (void**)&orig_WeaponShotFromServerDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564CBC), (void*)hook_VehicleChangeDoorStateDatagram_Read_42, (void**)&orig_VehicleChangeDoorStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2564F4C), (void*)hook_VehicleChangeFuelDatagram_Read_43, (void**)&orig_VehicleChangeFuelDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565070), (void*)hook_VehicleChangeLockStateDatagram_Read_44, (void**)&orig_VehicleChangeLockStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565220), (void*)hook_VehicleChangeMileageDatagram_Read_45, (void**)&orig_VehicleChangeMileageDatagram_Read);
    DobbyHook((void*)(libBase + 0x25653C4), (void*)hook_VehicleColorDatagram_Read_46, (void**)&orig_VehicleColorDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565570), (void*)hook_VehicleDrivingSchoolDatagram_Read_47, (void**)&orig_VehicleDrivingSchoolDatagram_Read);
    DobbyHook((void*)(libBase + 0x25656F4), (void*)hook_VehicleDroppedDownDatagram_Read_48, (void**)&orig_VehicleDroppedDownDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565878), (void*)hook_VehicleEmergencyLightsStateDatagram_Read_49, (void**)&orig_VehicleEmergencyLightsStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x25659FC), (void*)hook_VehicleEngineRunningDatagram_Read_50, (void**)&orig_VehicleEngineRunningDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565B48), (void*)hook_VehicleHornDatagram_Read_51, (void**)&orig_VehicleHornDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565CE4), (void*)hook_VehicleOdometerUpdateDatagram_Read_52, (void**)&orig_VehicleOdometerUpdateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565E18), (void*)hook_VehicleOwnDatagram_Read_53, (void**)&orig_VehicleOwnDatagram_Read);
    DobbyHook((void*)(libBase + 0x2565F9C), (void*)hook_VehicleParkingBrakeChangedCtsDatagram_Read_54, (void**)&orig_VehicleParkingBrakeChangedCtsDatagram_Read);
    DobbyHook((void*)(libBase + 0x2566120), (void*)hook_VehicleParkingBrakeChangedStcDatagram_Read_55, (void**)&orig_VehicleParkingBrakeChangedStcDatagram_Read);
    DobbyHook((void*)(libBase + 0x25662B8), (void*)hook_VehiclePositionTeleportDatagram_Read_56, (void**)&orig_VehiclePositionTeleportDatagram_Read);
    DobbyHook((void*)(libBase + 0x25663F8), (void*)hook_VehicleReputationDatagram_Read_57, (void**)&orig_VehicleReputationDatagram_Read);
    DobbyHook((void*)(libBase + 0x25664DC), (void*)hook_VehicleTemporaryDatagram_Read_58, (void**)&orig_VehicleTemporaryDatagram_Read);
    DobbyHook((void*)(libBase + 0x2566644), (void*)hook_VehicleTemporaryNetworkModel_Read_59, (void**)&orig_VehicleTemporaryNetworkModel_Read);
    DobbyHook((void*)(libBase + 0x25667E0), (void*)hook_VehicleTuningComponentChangeDatagram_Read_60, (void**)&orig_VehicleTuningComponentChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2566BC4), (void*)hook_VehicleTuningItemsDatagram_Read_61, (void**)&orig_VehicleTuningItemsDatagram_Read);
    DobbyHook((void*)(libBase + 0x2566DF4), (void*)hook_VehicleViewHornDatagram_Read_62, (void**)&orig_VehicleViewHornDatagram_Read);
    DobbyHook((void*)(libBase + 0x2566F74), (void*)hook_VehicleVinDatagram_Read_63, (void**)&orig_VehicleVinDatagram_Read);
    DobbyHook((void*)(libBase + 0x25670DC), (void*)hook_PedEnterToVehicleCTSDatagram_Read_64, (void**)&orig_PedEnterToVehicleCTSDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567328), (void*)hook_PedEnterToVehicleSTCDatagram_Read_65, (void**)&orig_PedEnterToVehicleSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567514), (void*)hook_PedExitFromVehicleCTSDatagram_Read_66, (void**)&orig_PedExitFromVehicleCTSDatagram_Read);
    DobbyHook((void*)(libBase + 0x25675E8), (void*)hook_PedExitFromVehicleSTCDatagram_Read_67, (void**)&orig_PedExitFromVehicleSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567758), (void*)hook_PedOpenVehicleDoorDatagram_Read_68, (void**)&orig_PedOpenVehicleDoorDatagram_Read);
    DobbyHook((void*)(libBase + 0x25679BC), (void*)hook_PedSwitchRejectVehicleSeatSTCDatagram_Read_69, (void**)&orig_PedSwitchRejectVehicleSeatSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567AF0), (void*)hook_PedSwitchVehicleSeatCTSDatagram_Read_70, (void**)&orig_PedSwitchVehicleSeatCTSDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567D18), (void*)hook_PedSwitchVehicleSeatSTCDatagram_Read_71, (void**)&orig_PedSwitchVehicleSeatSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2567EE4), (void*)hook_PedTryEnterToVehicleCTSDatagram_Read_72, (void**)&orig_PedTryEnterToVehicleCTSDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568130), (void*)hook_PedTryEnterToVehicleSTCDatagram_Read_73, (void**)&orig_PedTryEnterToVehicleSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568358), (void*)hook_PedTryExitFromVehicleCTSDatagram_Read_74, (void**)&orig_PedTryExitFromVehicleCTSDatagram_Read);
    DobbyHook((void*)(libBase + 0x25684C4), (void*)hook_PedTryExitFromVehicleSTCDatagram_Read_75, (void**)&orig_PedTryExitFromVehicleSTCDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568590), (void*)hook_RemovePedSeatDatagram_Read_76, (void**)&orig_RemovePedSeatDatagram_Read);
    DobbyHook((void*)(libBase + 0x25686DC), (void*)hook_ReplacePedSeatDatagram_Read_77, (void**)&orig_ReplacePedSeatDatagram_Read);
    DobbyHook((void*)(libBase + 0x25688DC), (void*)hook_HouseAvailabilityStateDatagram_Read_78, (void**)&orig_HouseAvailabilityStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568A5C), (void*)hook_HouseClassTypeDatagram_Read_79, (void**)&orig_HouseClassTypeDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568BF0), (void*)hook_HouseNameDatagram_Read_80, (void**)&orig_HouseNameDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568D58), (void*)hook_HouseParkingVisible_Read_81, (void**)&orig_HouseParkingVisible_Read);
    DobbyHook((void*)(libBase + 0x2568E80), (void*)hook_RequestCompleteQuestStepDatagram_Read_82, (void**)&orig_RequestCompleteQuestStepDatagram_Read);
    DobbyHook((void*)(libBase + 0x2568F0C), (void*)hook_RequestStartQuestDatagram_Read_83, (void**)&orig_RequestStartQuestDatagram_Read);
    DobbyHook((void*)(libBase + 0x256901C), (void*)hook_SetQuestElementDatagram_Read_84, (void**)&orig_SetQuestElementDatagram_Read);
    DobbyHook((void*)(libBase + 0x256918C), (void*)hook_UpdateQuestStepDatagram_Read_85, (void**)&orig_UpdateQuestStepDatagram_Read);
    DobbyHook((void*)(libBase + 0x25692FC), (void*)hook_PedBlockRotationDatagram_Read_86, (void**)&orig_PedBlockRotationDatagram_Read);
    DobbyHook((void*)(libBase + 0x2569468), (void*)hook_PedClothesChangedDatagram_Read_87, (void**)&orig_PedClothesChangedDatagram_Read);
    DobbyHook((void*)(libBase + 0x256962C), (void*)hook_PedClothesVariantsChangedDatagram_Read_88, (void**)&orig_PedClothesVariantsChangedDatagram_Read);
    DobbyHook((void*)(libBase + 0x25697A8), (void*)hook_PedCuffDatagram_Read_89, (void**)&orig_PedCuffDatagram_Read);
    DobbyHook((void*)(libBase + 0x2569A3C), (void*)hook_PedDeadDatagram_Read_90, (void**)&orig_PedDeadDatagram_Read);
    DobbyHook((void*)(libBase + 0x2569D70), (void*)hook_PedInteriorPositionDatagram_Read_91, (void**)&orig_PedInteriorPositionDatagram_Read);
    DobbyHook((void*)(libBase + 0x2569EB8), (void*)hook_PedOverrideClothesChangedDatagram_Read_92, (void**)&orig_PedOverrideClothesChangedDatagram_Read);
    DobbyHook((void*)(libBase + 0x256A088), (void*)hook_PedReviveDatagram_Read_93, (void**)&orig_PedReviveDatagram_Read);
    DobbyHook((void*)(libBase + 0x256A194), (void*)hook_PedStaminaStateDatagram_Read_94, (void**)&orig_PedStaminaStateDatagram_Read);
    DobbyHook((void*)(libBase + 0x256A528), (void*)hook_NicknameAdditionalDataDatagram_Read_95, (void**)&orig_NicknameAdditionalDataDatagram_Read);
    DobbyHook((void*)(libBase + 0x256A7C4), (void*)hook_NicknameColorDatagram_Read_96, (void**)&orig_NicknameColorDatagram_Read);
    DobbyHook((void*)(libBase + 0x256AA7C), (void*)hook_InputFloat_Read_97, (void**)&orig_InputFloat_Read);
    DobbyHook((void*)(libBase + 0x256AE00), (void*)hook_InputStateData_Read_98, (void**)&orig_InputStateData_Read);
    DobbyHook((void*)(libBase + 0x256B370), (void*)hook_InputVector3_Read_99, (void**)&orig_InputVector3_Read);
    DobbyHook((void*)(libBase + 0x256B478), (void*)hook_PlayerInputStateInfoDatagram_Read_100, (void**)&orig_PlayerInputStateInfoDatagram_Read);
    DobbyHook((void*)(libBase + 0x256B4D4), (void*)hook_UpdatePlayerInputStateInfoDatagram_Read_101, (void**)&orig_UpdatePlayerInputStateInfoDatagram_Read);
    DobbyHook((void*)(libBase + 0x256B584), (void*)hook_AttachedDisplacementSyncDatagram_Read_102, (void**)&orig_AttachedDisplacementSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x256BA50), (void*)hook_ElementFreezeDatagram_Read_103, (void**)&orig_ElementFreezeDatagram_Read);
    DobbyHook((void*)(libBase + 0x256BD08), (void*)hook_ElementModelDatagram_Read_104, (void**)&orig_ElementModelDatagram_Read);
    DobbyHook((void*)(libBase + 0x256BE40), (void*)hook_ElementPositionDatagram_Read_105, (void**)&orig_ElementPositionDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C0D8), (void*)hook_ElementPositionWithSnapDatagram_Read_106, (void**)&orig_ElementPositionWithSnapDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C2F8), (void*)hook_ElementRotationDatagram_Read_107, (void**)&orig_ElementRotationDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C460), (void*)hook_ElementWorldDatagram_Read_108, (void**)&orig_ElementWorldDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C558), (void*)hook_ObjectDisplacementSyncDatagram_Read_109, (void**)&orig_ObjectDisplacementSyncDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C770), (void*)hook_PlayerDriverLicenseChangeDatagram_Read_110, (void**)&orig_PlayerDriverLicenseChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C7F0), (void*)hook_PlayerEmploymentBookChangeDatagram_Read_111, (void**)&orig_PlayerEmploymentBookChangeDatagram_Read);
    DobbyHook((void*)(libBase + 0x256C8C0), (void*)hook_WeaponTizerEffectDatagram_Read_112, (void**)&orig_WeaponTizerEffectDatagram_Read);
}
