//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

namespace inet;

enum CDPTimerType
{
    UpdateTime = 0;
    HoldTime = 1;
    StartUp = 2;
    ODRInvalideTime = 3;
    ODRHoldTime = 4;
    ODRFlush = 5;
    ODRDefaultRouteInvalide = 6;
}

//CDP timers
message CDPTimer
{
    char timerType  @enum(CDPTimerType);//enum(CDPTimerType) = UpdateTime;
    int interfaceId;
}