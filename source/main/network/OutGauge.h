/*
    This source file is part of Rigs of Rods
    Copyright 2005-2012 Pierre-Michel Ricordel
    Copyright 2007-2012 Thomas Fischer

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

/// @file
/// @author Thomas Fischer (thomas{AT}thomasfischer{DOT}biz)
/// @date   17th of July 2011

#pragma once

#include "Application.h"

#ifdef _WIN32
    #include <Winsock2.h>
    #define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
    // Para usar sockets en linux
    #include <arpa/inet.h> // htons, inet_addr
    #include <netinet/in.h> // sockaddr_in
    #include <sys/types.h> // uint16_t
    #include <sys/socket.h> // socket, sendto
    #include <unistd.h> // close
    #define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif 

namespace RoR {

/// @addtogroup Network
/// @{

class OutGauge
{
public:

    OutGauge(void);

    void Connect();
    bool Update(float dt, ActorPtr truck);
    void Close();

private:

    bool working;
    float timer;
    int sockfd;

    

    // from LFS/doc/insim.txt
    enum
    {
        OG_SHIFT      = 1,           // key
        OG_CTRL       = 2,           // key
        OG_TURBO      = 8192,        // show turbo gauge
        OG_KM         = 16384,       // if not set - user prefers MILES
        OG_BAR        = 32768,       // if not set - user prefers PSI
    };

    enum
    {
        DL_SHIFT      = BITMASK(1),  // bit 0   - shift light
        DL_FULLBEAM   = BITMASK(2),  // bit 1   - full beam
        DL_HANDBRAKE  = BITMASK(3),  // bit 2   - handbrake
        DL_PITSPEED   = BITMASK(4),  // bit 3   - pit speed limiter
        DL_TC         = BITMASK(5),  // bit 4   - TC active or switched off
        DL_SIGNAL_L   = BITMASK(6),  // bit 5   - left turn signal
        DL_SIGNAL_R   = BITMASK(7),  // bit 6   - right turn signal
        DL_SIGNAL_ANY = BITMASK(8),  // bit 7   - shared turn signal
        DL_OILWARN    = BITMASK(9),  // bit 8   - oil pressure warning
        DL_BATTERY    = BITMASK(10), // bit 9   - battery warning
        DL_ABS        = BITMASK(11), // bit 10  - ABS active or switched off
        DL_SPARE      = BITMASK(12), // bit 11
        DL_NUM        = BITMASK(13)  // bit 14  - end
    };

    PACK (struct OutGaugePack
    {
        unsigned int   Time;            // time en milisegundos (para chequear el orden de los paquetes)
        char           Car_Name[32];    // Car's name
        int            Type;            // Tipo de vehiculo 

        float          Speed;           // m/s basado en el movimiento de las ruedas
        float          Velocity_X;      // Vector velocidad del auto
        float          Velocity_Y;
        float          Velocity_Z;
        
        float          GForces_Vertical;
        float          GForces_Sagital;
        float          GForces_Lateral;
        
        float          RPM;             // RPM
        int            Gear;            // Reverse:-1, Neutral:0, First:1...
        float          Throttle;        // 0 to 1
        float          Brake;           // 0 to 1
        float          Clutch;   
        float          SteeringAngle;   

        float          Height;
        float          Odometer;   
        
        unsigned int   ShowLights;      // Luces
        
        float          RPM_Max;         // Para seleccionar instrumento
        float          Speed_Max;       // Para seleccionar instrumento
    });
};

/// @}   //addtogroup Network

} // namespace RoR

