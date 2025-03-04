/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPEEDDETECTOR_H
#define SPEEDDETECTOR_H

#define SPDT_SAMPLINGRATE 1000              /// there is no point flooding calculations
#define SPDT_DETECTION_ERROR 0.70f          /// ex: 10% for a speed=7 => 0.7 max speed cheat
#define CHEAT_ALARMS_TO_TRIGGER_CHEAT 5     /// if x alarms stack up over time then it is time to kick the player

class LocationVector;
class Player;

//////////////////////////////////////////////////////////////////////////////////////////
///\todo  problem : some players advance more then others by using speed cheat
/// solution : check the amount of advance a player does over time to see if it is higher then should be
/// situations :
///        - player plays with mouse triggering a setfacing packet every 16 ms would flood us with incorrect values
///        - client sends us every 500 ms a movement update
///        - server makes speed changes -> need to reset detector
///        - server uses scripts over character -> delay detector
/// error recovery :
///        - speed change is detected then system resets state to start measuring again
///        - speed is measured over interval to avoid small value problems
///        - cheat detection has to stack up to trigger real cheat alarm
/// problems :
///        - it takes SPDT_WINDOW_SIZE*SPDT_SAMPLINGRATE at each speed change until it gives relevant data
///        - moving in a small area is not considered cheating (it is hard to make that)
//////////////////////////////////////////////////////////////////////////////////////////
class SpeedCheatDetector
{
    public:

        SpeedCheatDetector();

    void addSample(LocationVector v, int timestamp, float speed);
        /// update the detector with new values
        void AddSample(float x, float y, int stamp, float player_speed);
        /// test cheater status
        inline char IsCheatDetected() { return cheat_threat >= CHEAT_ALARMS_TO_TRIGGER_CHEAT; }
        /// delay then reset cheat detector
        void SkipSamplingUntil(int stamp);
        /// take actions against a cheater
        void ReportCheater(Player* _player);
        /// reset internal values on speed change
        void EventSpeedChange();

    private:

        float last_x, last_y;
        int last_stamp;
        signed char cheat_threat;           /// don't draw quick conclusions. If player is suspicious over time then kill him
        float last_used_speed;              /// we reset if speed changed since our last measure
        float bigest_hacked_speed_dif;
};

#endif // SPEEDDETECTOR_H
