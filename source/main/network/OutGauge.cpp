/*
    This source file is part of Rigs of Rods
    Copyright 2005-2012 Pierre-Michel Ricordel
    Copyright 2007-2012 Thomas Fischer
    Copyright 2013-2020 Petr Ohlidal

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

#include "OutGauge.h"

#include "ScrewProp.h"

#include "Application.h"
#include "Actor.h"
#include "ActorManager.h"
#include "DashBoardManager.h"
#include "EngineSim.h"
#include "RoRVersion.h"

#include <Ogre.h>

using namespace Ogre;
using namespace RoR;

OutGauge::OutGauge(void) :
     sockfd(-1)
    , timer(0)
    , working(false)
{
}

void OutGauge::Close(void)
{
    if (sockfd != 0)
    {
    #ifdef _WIN32
        closesocket(sockfd);
    #else
        close(sockfd);
    #endif
        sockfd = 0;
        working = false;
    }
}

void OutGauge::Connect()
{
    // Abro el socket UDP para enviar los datos de telemetria
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        LOG(String("[RoR|OutGauge] Error creating socket for OutGauge: ").append(strerror(errno)));
        return;
    }

    LOG("[RoR|OutGauge] Socket created successfully");
    working = true;
}

bool OutGauge::Update(float dt, ActorPtr truck)
{
    if (!working)
    {
        return false;
    }

    // Solo actualiza los datos dsp del intervalo configurado en ms
    timer += dt;
    if (timer < (0.001f * App::io_outgauge_delay->getFloat()))
    {
        return true;
    }
    timer = 0;

    // Armado del paquete a enviar
    OutGaugePack gd;
    memset(&gd, 0, sizeof(gd));

    gd.Time = Root::getSingleton().getTimer()->getMilliseconds();
    if (!truck)
    {
        // Sin Vehiculo
        strncpy(gd.Car_Name, "Ninguno", 31);
    }
    else
    {
        // Dentro del Vehiculo 
        
        strncpy(gd.Car_Name, truck->getTruckName().c_str(), 31);
        gd.Type = truck->getTruckType();   // Tipo de vehiculo
        float speed = truck->getWheelSpeed();
        gd.Speed = speed >= 0 ? speed : -speed;
        
        Ogre::Vector3 vel = truck->getVelocity();
        gd.Velocity_X = vel.x;
        gd.Velocity_Y = vel.y;
        gd.Velocity_Z = vel.z;

        Ogre::Vector3 GForces = truck->getGForces();
        gd.GForces_Vertical = GForces.x;
        gd.GForces_Sagital = GForces.y;
        gd.GForces_Lateral = GForces.z;

        gd.Height = truck->getHeightAboveGround();  // Altura o profundidad

        if (truck->ar_engine) 
        {
            // Vehiculo terrestre con motor 
            gd.RPM = truck->ar_engine->GetEngineRpm();
            gd.Gear = truck->ar_engine->GetGear(); 
            
            gd.Throttle = truck->ar_engine->GetAcceleration();
            gd.Brake = truck->ar_brake;
            gd.Clutch = truck->ar_engine->GetClutch();
            gd.SteeringAngle = truck->getSteeringAngle();

            gd.Odometer = truck->m_odometer_total;

            gd.ShowLights = 0;
            if (truck->ar_parking_brake)
                gd.ShowLights |= DL_HANDBRAKE;
            if (truck->getHeadlightsVisible())
                gd.ShowLights |= DL_FULLBEAM;
            if (truck->ar_engine->hasContact() && !truck->ar_engine->isRunning())
                gd.ShowLights |= DL_BATTERY;
            if (truck->ar_dashboard->_getBool(DD_SIGNAL_TURNLEFT))
                gd.ShowLights |= DL_SIGNAL_L;
            if (truck->ar_dashboard->_getBool(DD_SIGNAL_TURNRIGHT))
                gd.ShowLights |= DL_SIGNAL_R;
            if (truck->ar_dashboard->_getBool(DD_SIGNAL_WARNING))
                gd.ShowLights |= DL_SIGNAL_ANY;
            if (truck->tc_mode)
                gd.ShowLights |= DL_TC;
            if (truck->alb_mode)
                gd.ShowLights |= DL_ABS;

            gd.RPM_Max = truck->ar_engine->getMaxRPM();
            gd.Speed_Max = truck->ar_guisettings_speedo_max_kph;
        }
        else if (truck->ar_screwprops[0])
        {
            // Vehiculo maritimo con al menos 1 motor 
            gd.Throttle = truck->ar_screwprops[0]->getThrottle();
            gd.SteeringAngle = truck->ar_screwprops[0]->getRudder();
            // water speed
            Vector3 hdir = truck->GetCameraDir();
            gd.Speed = hdir.dotProduct(truck->ar_nodes[truck->ar_main_camera_node_pos].Velocity) * 1.9438f; // 1.943 = m/s in knots/s
        }

    }

    // Configura la direccion para enviar 
    struct sockaddr_in sendaddr;
    memset(&sendaddr, 0, sizeof(sendaddr));
    sendaddr.sin_family = AF_INET;
    sendaddr.sin_addr.s_addr = inet_addr(App::io_outgauge_ip->getStr().c_str());
    sendaddr.sin_port = htons(App::io_outgauge_port->getInt());

    sendto(sockfd, (const char*)&gd, sizeof(gd), 0, (const sockaddr *) &sendaddr, sizeof(sendaddr));

    return true;

}
