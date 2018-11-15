/***************************************************************************
**                                                                        **
**  Neslabus, controlling interface for Neslab thermostated bath through  **
**  Serial port for Qt Copyright (C) 2015 Jakub Klener                    **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU Lesser General Public License as        **
**  published by the Free Software Foundation, either version 2.1 of the  **
**  License, or (at your option) any later version.                       **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          **
**  GNU Lesser General Public License for more details.                   **
**                                                                        **
**  You should have received a copy of the GNU Lesser General Public      **
**  License along with this program.                                      **
**  If not, see http://www.gnu.org/licenses/.                             **
**                                                                        **
****************************************************************************
**           Author: Jakub Klener                                         **
**  Website/Contact: klener@karlov.mff.cuni.cz                            **
**             Date: 7. 4. 2015                                           **
**          Version: 1.0.0                                                **
****************************************************************************/

#include "neslab.h"

#ifdef VIRTUALSERIALPORT
#include <QDateTime>
#include <ctime>	// na seedovani generatoru nahodnych cisel
#include <QDebug>
#else  // VIRTUALSERIALPORT
#include <QSerialPort>
#include <QSerialPortInfo>
#endif // VIRTUALSERIALPORT

#include <QStringBuilder>

#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

/*!
    \file neslab.h

    \defgroup macros Preprocessor macros

    \def VIRTUALSERIALPORT
    \ingroup macros
    \brief This macro have to be defined before the neslab.h is included or at
    the top of neslab.h file to use VirtualSerialPort insted of QSerialPort
    class if you want to simulate thermostated %Neslab bath although it is not
    connected.
*/

/*!
    \namespace NeslabTraits
    \brief Contains Neslab traits
*/

using namespace NeslabTraits;

/*!
    \enum NeslabTraits::Command
    \brief This scoped enum defines the valid commands.

    They are ordered from the most important command (value 0) to the
    least one. The last entry is None which also determines the number
    of all valid commands.

    \sa Neslab::commandBuffer.

    \var SetOnOff
         \brief Set on off command.
    \var ReadStatus
         \brief Read status command.
    \var ReadAcknowledge
         \brief Read acknowledge command.
    \var SetLowTemperatureLimit
         \brief Set low temperature limit command.
    \var ReadLowTemperatureLimit
         \brief Read low temperature limit command.
    \var SetHighTemperatureLimit
         \brief Set high temperature limit command.
    \var ReadHighTemperatureLimit
         \brief Read high temperature limit command.
    \var SetHeatProportionalBand
         \brief Set heat proportional band command.
    \var ReadHeatProportionalBand
         \brief Read heat proportional band command.
    \var SetHeatIntegral
         \brief Set heat integral command.
    \var ReadHeatIntegral
         \brief Read heat integral command.
    \var SetHeatDerivative
         \brief Set heat derivative command.
    \var ReadHeatDerivative
         \brief Read heat derivative command.
    \var SetCoolProportionalBand
         \brief Set cool proportional band command.
    \var ReadCoolProportionalBand
         \brief Read cool proportional band command.
    \var SetCoolIntegral
         \brief Set cool integral command.
    \var ReadCoolIntegral
         \brief Read cool integral command.
    \var SetCoolDerivative
         \brief Set cool derivative command.
    \var ReadCoolDerivative
         \brief Read cool derivative command.
    \var ReadSetpoint
         \brief Read setpoint command.
    \var SetSetpoint
         \brief Set setpoint command.
    \var ReadExternalSensor
         \brief Read external sensor command.
    \var ReadInternalTemperature
         \brief Reat internal temperature command.
    \var None
         \brief This value determines the number of the commands.
*/

/*!
    \fn Command& NeslabTratis::operator++(Command &cmd)
    \brief     pre-increment operator which returns next lower priority Command
    enum value.
    \param cmd Command enum value.
    \return    Command enum value for command with next lower priority.
*/

/*!
    \fn operator++(Command &cmd, int dummyarg)
    \brief     post-increment operator which returns the same value as was in
    cmd.
    \param cmd Command enum value
    \param dummyarg dummy parameter used to resolve this operator from the
    pre-increment operator. See the c++ standard.
    \return    The same value as was cmd.
*/

/*!
    \enum NeslabTraits::PowerOnOff
    \brief This scoped enum determines the meaning of databytes for power on/off
    command.

    It is designed to use with PowerOnOffParm structure, which can provide an
    interface for power on/off command bytes.

    \sa Neslab::setOnOffCommand(), Neslab::onSetOnOff()

    \var d1_unitOnOff
        \brief switch unit on/off
    \var d2_sensorEnabled
        \brief enable/disable external temperature sensor
    \var d3_faultsEnabled
        \brief enable/disable the autopower off of the unit when fault occures
    \var d4_mute
        \brief stops alarm when beeping
    \var d5_autoRestart
        \brief autorestart?
    \var d6_01precDegCEnable
        \brief enables/disables precision of 0.01 °C, otherwise 0.1 is used
    \var d7_fullRangeCoolEnable
        \brief enables/disables cooling over 50 °C.
    \var d8_serialCommEnable
        \brief enbales/disables serial port communication. When disabled, it is
        possible to enable it only with the button on the bath.
*/

/*!
    \struct NeslabTraits::PowerOnOffParm
    \brief The class that provides an interface for power on/off command bytes.

    This class can be used to store data bytes for power on/off command. This
    bytes is intended to be accessed with NeslabTraits::PowerOnOff enum values.

    The values of particular bytes can be only 0 for false (or is false in bath
    response), 1 for true (or is true in bath response) and 2 for do not change
    the value.

    Example of the use inside of slot for enableSensorCheck check box
    QCheckBox::StateChanged signal. powerOnOffParm_ is pointer to the
    internally stored PowerOnOffParm which contains the actual state and neslab
    is pointer to the applications Neslab object.
    \code
    void onEnableSensoCheckStateChanged()
    {
        NeslabTraits::PowerOnOffParm powerOnOffParm; // constructs the PowerOnOffParm object with default
        // values of each byte set to 2 (which means do not change the state)

        if (enableSensorCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d2_sensorEnabled] = 1; // sets the command to enable
            // external sensor
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d2_sensorEnabled] = 0; // sets the command to disable
            // external sensor

        neslab->setOnOffCommand(powerOnOffParm); // sends the command to neslab class.
    }
    \endcode

    \sa Neslab, Neslab::setOnOffCommand()
*/

/*!
    \fn NeslabTraits::PowerOnOffParm::PowerOnOffParm()
    \brief Constructs the NeslabTraits object

    Sets all the stored bytes to 2, which means do not change state.
*/

/*!
    \fn unsigned char & NeslabTraits::PowerOnOffParm::operator[](PowerOnOff d)
    \brief Enables acces to the stored bytes through the
    NeslabTraits::PowerOnOff enum.
*/

/*!
    \fn unsigned char & NeslabTraits::PowerOnOffParm::operator[](int d)
    \brief Enables acces to the stored bytes through int param.
*/

/*!
    \fn const unsigned char & NeslabTraits::PowerOnOffParm::operator[](PowerOnOff d) const
    \brief Enables acces to the stored bytes through the
    NeslabTraits::PowerOnOff enum.
*/

/*!
    \fn const unsigned char & NeslabTraits::PowerOnOffParm::operator[](int d) const
    \brief Enables acces to the stored bytes through int param.
*/

/*!
    \fn void NeslabTraits::PowerOnOffParm::erase()
    \brief Resets all the stored bytes to 2, which means do not change state.
*/

/*!
    \fn const unsigned char * NeslabTraits::PowerOnOffParm::bytes()
    \brief Returns stored byte array.

    \warning
    Returns pointer to internally stored array. Ensure that the PowerOnOffParm
    object is still valid when the data are used.
*/

/*!
    \var NeslabTraits::PowerOnOffParm::bytes_
    \brief Array of bytes containing the command data.
*/

/*!
    \namespace NeslabTraits::BathStatusTraits
    \brief This namespace contains enums of bitfield flags which determines the
    meanings of data received after the Neslab::readStatusCommand()

    These data bytes are meaned to be stored in NeslabTraits::BathStatus
*/

/*!
    \enum NeslabTraits::BathStatusTraits::d1

    \brief This scoped enum determines the meaning of the first data byte
    received after the Neslab::readStatusCommand()

    \sa BathStatusTraits BathStatus
*/

/*
    \var None = 0
        \brief NeslabTraits::BathStatusTraits::d1::None
    \var RTD1OpenFault
        \brief RTD1OpenFault = 1 << 7
    \var RTD1ShortedFault
        \brief RTD1ShortedFault = 1 << 6
    \var RTD1Open
        \brief RTD1Open = 1 << 5
    \var RTD1Shorted
        \brief RTD1Shorted = 1 << 4
    \var RTD3OpenFault
        \brief RTD3OpenFault = 1 << 3
    \var RTD3ShortedFault
        \brief RTD3ShortedFault = 1 << 2
    \var RTD3Open
        \brief RTD3Open = 1 << 1
    \var RTD3Shorted
        RTD3Shorted = 1 << 0
*/

/*!
    \enum NeslabTraits::BathStatusTraits::d2
    \brief This scoped enum determines the meaning of the second data byte
    received after the Neslab::readStatusCommand()

    \sa BathStatusTraits BathStatus
*/
/*
    \var d2::None         = 0
    \var RTD2OpenFault    = 1 << 7
    \var RTD2ShortedFault = 1 << 6
    \var RTD2OpenWarn     = 1 << 5
    \var RTD2ShortedWarn  = 1 << 4
    \var RTD2Open         = 1 << 3
    \var RTD2Shorted      = 1 << 2
    \var RefrigHighTemp   = 1 << 1
    \var HTCFault         = 1 << 0
*/

/*!
    \enum NeslabTraits::BathStatusTraits::d3
    \brief This scoped enum determines the meaning of the third data byte
    received after the Neslab::readStatusCommand()

    \sa BathStatusTraits BathStatus
*/
/*
    \var None               = 0
    \var HighFixedTempFault = 1 << 7
    \var LowFixedTempFault  = 1 << 6
    \var HighTempFault      = 1 << 5
    \var LowTempFault       = 1 << 4
    \var LowLevelFault      = 1 << 3
    \var HighTempWarn       = 1 << 2
    \var LowTempWarn        = 1 << 1
    \var LowLevelWarn       = 1 << 0
*/

/*!
    \enum NeslabTraits::BathStatusTraits::d4
    \brief This scoped enum determines the meaning of the fourth data byte
    received after the Neslab::readStatusCommand()

    \sa BathStatusTraits BathStatus
*/
/*
    \var None         = 0
    \var BuzzerOn     = 1 << 7
    \var AlarmMuted   = 1 << 6
    \var UnitFaulted  = 1 << 5
    \var UnitStopping = 1 << 4
    \var UnitOn       = 1 << 3
    \var PumpOn       = 1 << 2
    \var CompressorOn = 1 << 1
    \var HeaterOn     = 1 << 0
*/

/*!
    \enum NeslabTraits::BathStatusTraits::d5
    \brief This scoped enum determines the meaning of the fifth data byte
    received after the Neslab::readStatusCommand().

    \sa BathStatusTraits BathStatus
*/
/*
    \var None            = 0
    \var RTD2Controlling = 1 << 7
    \var HeatLEDFlashing = 1 << 6
    \var HeatLEDOn       = 1 << 5
    \var CoolLEDFlashing = 1 << 4
    \var CoolLEDOn       = 1 << 3
    \var Other1          = 1 << 2
    \var Other2          = 1 << 1
    \var Other3          = 1 << 0
*/

/*!
    \fn d1 NeslabTraits::BathStatusTraits::operator|(d1 a, d1 b)
        \brief Bitwise OR operator on NeslabTraits::BathStatusTraits::d1
    \fn d1 NeslabTraits::BathStatusTraits::operator&(d1 a, d1 b)
        \brief Bitwise AND operator on NeslabTraits::BathStatusTraits::d1
    \fn d1 NeslabTraits::BathStatusTraits::operator~(d1 a)
        \brief Bitwise NOT operator on NeslabTraits::BathStatusTraits::d1
    \fn d1& NeslabTraits::BathStatusTraits::operator|=(d1& a, d1 b)
        \brief Bitwise compound OR assignment operator on
        NeslabTraits::BathStatusTraits::d1
    \fn d1& NeslabTraits::BathStatusTraits::operator&=(d1& a, d1 b)
        \brief Bitwise compound AND assignment operator on
        NeslabTraits::BathStatusTraits::d1
    \fn d2 NeslabTraits::BathStatusTraits::operator|(d2 a, d2 b)
        \brief Bitwise OR operator on NeslabTraits::BathStatusTraits::d2
    \fn d2 NeslabTraits::BathStatusTraits::operator&(d2 a, d2 b)
        \brief Bitwise AND operator on NeslabTraits::BathStatusTraits::d2
    \fn d2 NeslabTraits::BathStatusTraits::operator~(d2 a)
        \brief Bitwise NOT operator on NeslabTraits::BathStatusTraits::d2
    \fn d2& NeslabTraits::BathStatusTraits::operator|=(d2& a, d2 b)
        \brief Bitwise compound OR assignment operator on
        NeslabTraits::BathStatusTraits::d2
    \fn d2& NeslabTraits::BathStatusTraits::operator&=(d2& a, d2 b)
        \brief Bitwise compound AND assignment operator on
        NeslabTraits::BathStatusTraits::d2
    \fn d3 NeslabTraits::BathStatusTraits::operator|(d3 a, d3 b)
        \brief Bitwise OR operator on NeslabTraits::BathStatusTraits::d3
    \fn d3 NeslabTraits::BathStatusTraits::operator&(d3 a, d3 b)
        \brief Bitwise AND operator on NeslabTraits::BathStatusTraits::d3
    \fn d3 NeslabTraits::BathStatusTraits::operator~(d3 a)
        \brief Bitwise NOT operator on NeslabTraits::BathStatusTraits::d3
    \fn d3& NeslabTraits::BathStatusTraits::operator|=(d3& a, d3 b)
        \brief Bitwise compound OR assignment operator on
        NeslabTraits::BathStatusTraits::d3
    \fn d3& NeslabTraits::BathStatusTraits::operator&=(d3& a, d3 b)
        \brief Bitwise compound AND assignment operator on
        NeslabTraits::BathStatusTraits::d3
    \fn d4 NeslabTraits::BathStatusTraits::operator|(d4 a, d4 b)
        \brief Bitwise OR operator on NeslabTraits::BathStatusTraits::d4
    \fn d4 NeslabTraits::BathStatusTraits::operator&(d4 a, d4 b)
        \brief Bitwise AND operator on NeslabTraits::BathStatusTraits::d4
    \fn d4 NeslabTraits::BathStatusTraits::operator~(d4 a)
        \brief Bitwise NOT operator on NeslabTraits::BathStatusTraits::d4
    \fn d4& NeslabTraits::BathStatusTraits::operator|=(d4& a, d4 b)
        \brief Bitwise compound OR assignment operator on
        NeslabTraits::BathStatusTraits::d4
    \fn d4& NeslabTraits::BathStatusTraits::operator&=(d4& a, d4 b)
        \brief Bitwise compound AND assignment operator on
        NeslabTraits::BathStatusTraits::d4
    \fn d5 NeslabTraits::BathStatusTraits::operator|(d5 a, d5 b)
        \brief Bitwise OR operator on NeslabTraits::BathStatusTraits::d5
    \fn d5 NeslabTraits::BathStatusTraits::operator&(d5 a, d5 b)
        \brief Bitwise AND operator on NeslabTraits::BathStatusTraits::d5
    \fn d5 NeslabTraits::BathStatusTraits::operator~(d5 a)
        \brief Bitwise NOT operator on NeslabTraits::BathStatusTraits::d5
    \fn d5& NeslabTraits::BathStatusTraits::operator|=(d5& a, d5 b)
        \brief Bitwise compound OR assignment operator on
        NeslabTraits::BathStatusTraits::d5
    \fn d5& NeslabTraits::BathStatusTraits::operator&=(d5& a, d5 b)
        \brief Bitwise compound AND assignment operator on
        NeslabTraits::BathStatusTraits::d5
*/

/*!
    \struct NeslabTraits::BathStatus
    \brief This structure stores the data received after the
    Neslab::readStatusCommand()

    The meaning of particular bits is stored in enums contained in namespace
    NeslabTraits::BathStatusTraits.

    \fn BathStatus::BathStatus()
    \brief Constructs the BathStatus object.

    \var BathStatus::d1
        \brief 1st data byte.
    \var BathStatus::d2
        \brief 2nd data byte.
    \var BathStatus::d3
        \brief 3rd data byte.
    \var BathStatus::d4
        \brief 4th data byte.
    \var BathStatus::d5
        \brief 5th data byte.
*/

/*!
    \struct NeslabTraits::ComPortParams
    \brief Structure used to store parameters of available ports.

    \sa Neslab::availablePorts()

    \var NeslabTraits::ComPortParams::portName
        \brief com port name
    \var NeslabTraits::ComPortParams::description
        \brief com port description
    \var NeslabTraits::ComPortParams::manufacturer
        \brief com port manufacturer
*/

/*!
    \class Neslab
    \brief The class that provides the interface for %Neslab thermostated bath
    controlling through serial port.

    \remark reentrant, thread-safe

    The %Neslab bath communication is based on master slave concept where the
    master (controlling computer) sends command to the bath and the bath sends
    answer for this command. The commands are divided to two categories. Those
    which send no data bytes and contains request for some parameters (all read
    commands) and those which send some data to the bath and do not expect any
    new information in response. The exception is the set on/off command, which
    gets current bath state in received data bytes even though it can be also
    used to set some bath parameters.

    The Neslab class works with asynchronous concept of serial port based on
    signals and slots. It sends command to the bath and then waits in non
    blocking way for the signal that new data on the serial port is available.

    It can also run without any serial port using VirtualSerialPort instead of
    QSerialPort class. Define the ::VIRTUALSERIALPORT preprocessor macro before
    the neslab.h is included or at the top of the neslab.h

    The commands inside the class have all their own priority. The most
    important command is determined by the first member of
    NeslabTraits::Command enum and the least important is the last member. The
    very last member of this enum is NeslabTraits::Command::None, which stands
    for number of valid commands.

    If someone sends the command, firstly it is checked if the computer is
    connected to the %Neslab thermostated bath (see the connectNeslab() slot).
    If not, it do not do anything. Then it is checked wheather there is any
    command in process and if not, it directly sends it to the bath calling
    private method with prefix "send" and emits sendingCommand() signal (so
    this signal is emited only when there is not command in the buffer
    present). If there is some command in process, the corresponding value for
    this command is set to true in the Neslab::commandBuffer array. It means,
    that no command can repeat in the buffer.

    The class is thread-safe, but the QSerialPort cannot be controlled by
    more than one thread, so it is necessary to transfer all communication from
    the calling thread to the Neslab class containing thread. It is done by
    emiting connectNeslabInThread() and sendToSerial() internal signals used in
    connectNeslab() and send command methods, which are connected to the
    onConnectNeslabInThread() and onSendToSerial() private slots, respectively.

    Then it waits for the response from the bath, which triggers the
    onReadyData() method, which checks whather the response is valid and calls
    corresponding function (the naming convention is that the name of command
    is stripped from the word "command" and prefixed by "on") which process the
    response. This function is selected according to the value stored in
    Neslab::lastCommand during the sending of the command. Then it starts
    Neslab::waitingTimer, which signleshots after Neslab::cmdDelay static
    constant ms and which is connected to the onWaitingFinished() method and
    delays the consecutive command sending to the bath because of electronic
    restrictions of communication.

    Then the sendCommand() private method is called, which controlls, if there
    is any commands in the Neslab::commandBuffer. It loops through this buffer
    from the most important command to the least one and if it encounters any
    command waiting, it stops further searching and sends it to the bath. If
    the buffer is empty (has all values set to false), the gotResponse() signal
    is emited. So, it means, that the bath sent correct response for all
    requested commands and there are no waiting commands in the buffer.

    In send command methods (and also commands slots in the case, that there is
    no command in the buffer at the time of command sending), it is possible to
    set, wheather you want to accept another same command to the buffer or not
    when the current command is sending setting its Neslab::commandBuffer
    vaule. If it is set to true during the waiting for response from bath and
    set to false after correct response is received (do not forget to set it to
    false in corresponding method called, after response received), then it
    cannot be added to the buffer during the waiting for the bath response.
    This approach was used for all read commands. The oposit approach, when the
    appropriate Neslab::commandBuffer value is already set to false in the
    sending command phase is used for all set commands.

    If the response from the bath is not valid, the last command is resent.
    This is repeated until it reaches Neslab::initTrialCount static constant
    trials, when the connectionFailed() signal is emited.
*/

// initialization of static constants

/*! \brief Default name of used serial port. */
const QString Neslab::defaultComPortName = "COM2";

/*! \brief QString hexadecimal code of the first 3 bytes of command. It is
    converted to bytes (unsigned char) and prepedned to the commands stored in
    bCommands. See fillCommandsArrays() for more details.*/
const QString Neslab::commandBase = "CA 00 01 ";

/*! \brief QString hexadecimal code of read command. */
const QString Neslab::rExtSensorCmd            = "21";
/*! \brief QString hexadecimal code of set setpoint command. */
const QString Neslab::wSetPointCmd             = "F0";
/*! \brief QString hexadecimal code of read setpoint command. */
const QString Neslab::rSetPointCmd             = "70";

/*! \brief QString hexadecimal code of read status command. */
const QString Neslab::rStatusCmd               = "09";
/*! \brief QString hexadecimal code of read read acknowledge command. */
const QString Neslab::rAcknowledgeCmd          = "00";
/*! \brief QString hexadecimal code of read internal temperature command. */
const QString Neslab::rInternalTCmd            = "20";
/*! \brief QString hexadecimal code of read low temperature limit command. */
const QString Neslab::rLowTLimCmd              = "40";
/*! \brief QString hexadecimal code of read high temperature limit command. */
const QString Neslab::rHighTLimCmd             = "60";
/*! \brief QString hexadecimal code of read heat proportional band comand. */
const QString Neslab::rHeatProportionalBandCmd = "71";
/*! \brief QString hexadecimal code of read heat integral command. */
const QString Neslab::rHeatIntegraCmd          = "72";
/*! \brief QString hexadecimal code of read heat derivative command. */
const QString Neslab::rHeatDerivativeCmd       = "73";
/*! \brief QString hexadecimal code of read cool proportional band command. */
const QString Neslab::rCoolProportionalBandCmd = "74";
/*! \brief QString hexadecimal code of read cool integral command. */
const QString Neslab::rCoolIntegraCmd          = "75";
/*! \brief QString hexadecimal code of read cool derivative command. */
const QString Neslab::rCoolDerivativeCmd       = "76";

/*! \brief QString hexadecimal code of set low temperature limit command. It
    must be lower than setpoint. */
const QString Neslab::wLowTLimCmd              = "C0";
/*! \brief QString hexadecimal code of set high temperature limit command. It
    must be higher than setpoint. */
const QString Neslab::wHigTLimCmd              = "E0";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.1-99.9 range. */
const QString Neslab::wHeatProportionalBandCmd = "F1";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.00-9.99 range. */
const QString Neslab::wHeatIntegraCmd          = "F2";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.00-5.00 range. */
const QString Neslab::wHeatDerivativeCmd       = "F3";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.1-99.9 range. */
const QString Neslab::wCoolProportionalBandCmd = "F4";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.00-9.99 range. */
const QString Neslab::wCoolIntegraCmd          = "F5";
/*! \brief QString hexadecimal code of set heat proportional band command. It
    must be in the 0.00-5.00 range. */
const QString Neslab::wCoolDerivativeCmd       = "F6";

/*! \brief QString hexadecimal code of on/off command. Always 8 bytes, each byte
    can take the value 0 setting a corresponding property to false, 1 for true
    and 2 means not to change the property. Example of command, which turns the
    unit on: CA 00 01 81 08 01 02 02 02 02 02 02 02 cs */
const QString Neslab::wOnOffCmd                 = "81";
/*! \brief QString hexadecimal code of message returned from bath when bad
    command is received. The full error message would be commandBase + this +
    command byte of bad command (see strCommands for more details) + checkSum
    (see CheckSum(const QString&) for more details.)

    /sa badCheckSumCmd*/
const QString Neslab::badCommandCmd            = "0F 02 01";
/*! \brief QString hexadecimal code of message returned from bath when bad
    checksum is received. The full error message would be commandBase + this +
    command byte of bad command (see strCommands for more details) + checkSum
    (see CheckSum(const QString&) for more details.) */
const QString Neslab::badCheckSumCmd           = "0F 02 03";
/*! \brief This static constant determines the longest time in ms to be waited
    for bath response after a command was sended.

    If this time is exceeded, the command is treated as
    unsuccesfull and onNoResponse() is executed. The cmdTimer private member is
    used to trigger the end of the waiting. This constant has proposed value of
    1000 ms in the bath manual. */
const int Neslab::waitForResponse = 1000;
/*! \brief This static constant determines delay after response of the bath and
    before the next command can be executed. The waitingTimer private member is
    used to triger the end of delay. This constant has proposed at least 5 ms in
    the bath manual. */
const int Neslab::cmdDelay = 10;
/*! \brief This static constant determines the number of trials to send the
    command. The command is regarded as successful only if the bath get response
    and the response is valid.
    \sa validateResponse(const unsigned char *, int, NeslabTraits::Command),
    sendCommand(), onReadyData(), onNoResponse() */
const int Neslab::initTrialCount = 3;
/*! \brief This static constant determines the maximum number, which can be
    send. This number is determined by the fact, that the numerical data is sent
    in 2 bytes. */
const int Neslab::maxNumber = 256 * 256;
/*! \brief This static constant used to recognize negative numbers from positive
    numbers.

    Neslab bath sends negative numbers such that the value of negative number
    is subtracted from the maximum number value, so I proposed the border
    between pozitive and negative numbers in the middle. But this value was not
    confirmed experimentally. */
const int Neslab::negativeNumbersLimit = Neslab::maxNumber / 2;
/*! \brief This static constant determines the default value of heat
    proportional band, which can be restored by the
    Neslab::restoreDefaultSettingsCommand() function. These values were taken
    from the %Neslab bath manual.
    \sa Neslab::defaultHeatI, Neslab::defaultHeatD, Neslab::defaultCoolP,
    Neslab::defaultCoolI, Neslab::defaultCoolD */
const double Neslab::defaultHeatP = 0.6;
/*! \brief This static constant determines the default value of heat integral,
    which can be restored by the Neslab::restoreDefaultSettingsCommand()
    function. These values were taken from the %Neslab bath manual.
    \sa Neslab::defaultHeatP, Neslab::defaultHeatD, Neslab::defaultCoolP,
    Neslab::defaultCoolI, Neslab::defaultCoolD */
const double Neslab::defaultHeatI = 0.6;
/*! \brief This static constant determines the default value of heat derivative,
    which can be restored by the Neslab::restoreDefaultSettingsCommand()
    function. These values were taken from the %Neslab bath manual.
    \sa Neslab::defaultHeatP, Neslab::defaultHeatI, Neslab::defaultCoolP,
    Neslab::defaultCoolI, Neslab::defaultCoolD */
const double Neslab::defaultHeatD = 0.0;
/*! \brief This static constant determines the default value of cool
    proportional band, which can be restored by the
    Neslab::restoreDefaultSettingsCommand() function. These values were taken
    from the %Neslab bath manual.
    \sa Neslab::defaultHeatP, Neslab::defaultHeatI, Neslab::defaultHeadD,
    Neslab::defaultCoolI, Neslab::defaultCoolD */
const double Neslab::defaultCoolP = 0.6;
/*! \brief This static constant determines the default value of cool integral,
    which can be restored by the Neslab::restoreDefaultSettingsCommand()
    function. These values were taken from the %Neslab bath manual.
    \sa Neslab::defaultHeatP, Neslab::defaultHeatI, Neslab::defaultHeadD,
    Neslab::defaultCoolP, Neslab::defaultCoolD */
const double Neslab::defaultCoolI = 0.6;
/*! \brief This static constant determines the default value of cool derivative,
    which can be restored by the Neslab::restoreDefaultSettingsCommand()
    function. These values were taken from the %Neslab bath manual.
    \sa Neslab::defaultHeatP, Neslab::defaultHeatI, Neslab::defaultHeatD,
    Neslab::defaultCoolP, Neslab::defaultCoolI */
const double Neslab::defaultCoolD = 0.0;


// initialization of static member variables
/*!
    \brief This static member variable contains a manger class for the mutex
    for the serial port information. It manages its destruction after the
    programm is finished and provides the possibility to forward declare the
    QMutex class. It is probably not necessary to have such complicated
    solution, because most operating systems frees automatically memory
    after application is finished and for static variables, there are not
    memory leaks possible, but for sure...
    \sa Neslab::SerialPortInfoMutexManager,
    Neslab::SerialPortInfoMutexManager::getMutex()
 */
Neslab::MutexManager Neslab::serialPortInfoMutexManager;

/*!
    \brief Array of 4 byte (3 leading bytes and one command byte) commands used
    to control the bath.

    It is filled by fillCommandsArrays() static method, the first command is
    the command with the most important priority, the last is the least
    important. They should be accessed using the NeslabTraits::Command enum
    values.

    Example, which shows how to build the whole command read status:

    \code
    int n = 6; // number of command bytes
    unsigned char cmd[n]; // array of command bytes

    // copying the first two bytes of command to command byte array
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadStatus)][ii];
    cmd[4] = 0; // 5th byte specifies the number of data bytes. For read
    // commands, there is no one.
    cmd[5] = Neslab::checkSum(cmd, 5); // the last byte contains check sum.
    \endcode
*/
unsigned char Neslab::bCommands[static_cast<int>(Command::None)][4];
/*!
    \brief Array of QString representation of command bytes used to control the
    bath.

    To assemble the full command, it is necessary to prepend the commandBase
    and append byte, which contains number of data bytes, folowed bytes
    containing data and ended with checksum (See CheckSum(const unsigned char,
    int)). It is filled by fillCommandsArrays() static method, the first
    command is the command with the most important priority, the last is the
    least important. They should be accessed using the NeslabTraits::Command
    enum values. Example, which shows how to build the whole command Read
    status:
    \code
    QString strCmd(commandBase);
    int n = 0; // number of data bytes
    strCmd.append(strCommands[static_cast<int>(Command::ReadStatus)])
          .append(QString(" %1 ").arg(n, 2, 16, QChar('0')).toUpper());
    strCmd.append(checkSum(strCmd));
    \endcode
*/
QString Neslab::strCommands[static_cast<int>(Command::None)];

/*!
  \brief Creates a new Neslab instance and sets the default values.
*/
Neslab::Neslab(QObject *parent) :
    QObject(parent)
{
    // fills arrays containing binary (bCommands) and QString (strCommands)
    // commands from static constants
    fillCommandsArrays();

    // creating and connecting QTimers.
    cmdTimer = new QTimer(this); // waiting for response timer
    connect(cmdTimer, &QTimer::timeout, this, &Neslab::onNoResponse);
    waitingTimer = new QTimer(this); // waiting between commands timer.
    connect(waitingTimer, &QTimer::timeout, this, &Neslab::onWaitingFinished);
    // connecting signal, which provides the transfer of the starting of the
    // timer to the Neslab containing thread.
    connect(this, &Neslab::startCmdTimer, this, &Neslab::onStartCmdTimer);

    mutex = new QMutex;

    // private variables initialisation
    lastCommand = NeslabTraits::Command::None;
    connected_ = false;
    connecting_ = false;
    switchingOn_ = false;
    switchingOff_ = false;
    statusUpdatedAfterOnOffCmd_ = false;
    waitingForNextCommand_ = false;
    wOnOffParm_.erase();
    rOnOffParm_.erase();
    for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii)
        commandBuffer[ii] = false;
    comPortName_ = defaultComPortName;

    // connecting signal, which provides the transfer of the sending of the
    // command to the serial port to the Neslab containing thread.
    connect(this, &Neslab::sendToSerial, this, &Neslab::onSendToSerial);
    connect(this, &Neslab::connectNeslabInThread, this, &Neslab::onConnectNeslabInThread);

    // serial port initialisation
#ifdef VIRTUALSERIALPORT
    serialPort = new VirtualSerialPort(this);
    connect(serialPort, &VirtualSerialPort::readyRead, this, &Neslab::onReadyData);
#else // VIRTUALSERIALPORT
    serialPort = new QSerialPort(this);
    connect(serialPort, &QSerialPort::readyRead, this, &Neslab::onReadyData);
#endif // VIRTUALSERIALPORT
}

/*!
   \brief This function gets the name of COM port.
   \sa connectNeslab(), setComPortName(), Neslab::comPortName_, Neslab::serialPort
 */
const QString &Neslab::comPortName()
{
    QMutexLocker locker(mutex);
    return comPortName_;
}

/*!
   \brief This function sets the name of COM port, and disconnects it.
   \param name The new name of COM port.
   \sa connectNeslab(), comPortName(), Neslab::comPortName_, Neslab::serialPort
*/
void Neslab::setComPortName(const QString &name)
{
    QMutexLocker locker(mutex);
    if (comPortName_ != name) {
        comPortName_ = name;
        connected_ = false;
        serialPort->close();
        locker.unlock();
        emit disconnected();
    }
}

/*!
   \brief Gets available serial ports and their parameters.
   \return QList containing information about available ports
   \sa NeslabTraits::ComPortParams
 */
QList<ComPortParams> Neslab::availablePorts()
{
    QList<ComPortParams> comPortParamsList;
#ifdef VIRTUALSERIALPORT
    // random number generation, to simulate different port states
    std::default_random_engine engine;
    engine.seed(time(0));
    std::uniform_int_distribution<int> dist(0,0);

    ComPortParams comPortParams;

    QMutexLocker(serialPortInfoMutexManager.getMutex());
    if (!dist(engine)) {
        comPortParams.portName = "COM1";
        comPortParams.description = "VirtualSerialPort";
        comPortParams.manufacturer = "VirtualSerialPort";
        comPortParamsList.append(comPortParams);
    }
    if (!dist(engine)) {
        comPortParams.portName = "COM2";
        comPortParams.description = "VirtualSerialPort";
        comPortParams.manufacturer = "VirtualSerialPort";
        comPortParamsList.append(comPortParams);
    }
    if (!dist(engine)) {
        comPortParams.portName = "COM3";
        comPortParams.description = "VirtualSerialPort";
        comPortParams.manufacturer = "VirtualSerialPort";
        comPortParamsList.append(comPortParams);
    }
#else // VIRTUALSERIALPORT
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ComPortParams comPortParams;
        comPortParams.portName = info.portName();
        comPortParams.description = info.description();
        comPortParams.manufacturer = info.manufacturer();
        comPortParamsList.append(comPortParams);
    }
#endif // VIRTUALSERIALPORT
    return comPortParamsList;
}

/*!
   \brief Indicates wheather the bath is connected.
   \return true if bath is connected
   \sa connectNesalab(), connected_
 */
bool Neslab::isConnected() const
{
    QMutexLocker locker(mutex);
    return connected_;
}

/*!
   \brief Indicates wheather the bath is turned on.
   \param state true if the bath is turned on as it was acquired from the last
   sendSetOnOffCommand() or sendReadStatusCommand().
   \return true if status of bath was updated after last sending of
   sendSetOnOffCommand() and so the value of state parameter is valid.

   This overload of the function do not send any command to the bath and
   returns last received bath state. If the state was somehow changed during
   the time of last received answer after sendSetOnOffCommand() (for example
   manualy directly on the bath unit), the state is not up to date. It is also
   possible to use isTurnedOn() slot, which controls the unit state by the
   setOnOffCommand().
   \sa turnOn(), turnOff()
 */
bool Neslab::isTurnedOn(bool *state) const
{
    QMutexLocker locker(mutex);
    *state = (rOnOffParm_[PowerOnOff::d1_unitOnOff] == 1);
    if (statusUpdatedAfterOnOffCmd())
        return true;
    else
        return false;
}

/*!
   \brief Gets bath status, which was received in response to the last
   sendReadStatusCommand() calling.
   \param status is pointer to the bath status. It have to point to initialized
   memory before it is submited as a parameter to this function
   \return true if the response after last sendSetOnOffCommand() was already
   received, because this command can change the status (the value of
   statusUpdatedAfterOnOffCmd()).
 */
bool Neslab::getBathStatus(BathStatus *status) const
{
    QMutexLocker locker(mutex);
    *status = bathStatus_;
    if (statusUpdatedAfterOnOffCmd())
        return true;
    else
        return false;
}

/*!
   \brief Gets bath parameters, which were received after last
   sendSetOnOffCommand() calling.
   \return bath parameters.
   \sa rOnOffParm_
 */
PowerOnOffParm Neslab::getOnOffParms() const
{
    QMutexLocker locker(mutex);
    return rOnOffParm_;
}

/*!
   \brief Gets wheather the external sensor is used. This information was
   received in response to the last sendReadStatusCommand() calling.
   \param state is pointer to the Boolean variable. It have to point to
   initialized memory before it is submited as a parameter to this function
   \return true if the response after last sendSetOnOffCommand() was already
   received, because this command can change the state (the value of
   statusUpdatedAfterOnOffCmd()).
 */
bool Neslab::isExternalSensorUsed(bool *state) const
{
    QMutexLocker locker(mutex);
    *state = (rOnOffParm_[PowerOnOff::d2_sensorEnabled] == 1);
    if (statusUpdatedAfterOnOffCmd())
        return true;
    else
        return false;
}

// the description of signals follows

/*!
    \fn Neslab::sendingCommand()
        \brief Emited when sending new command to bath.
        \warning The internal locker is being locked when this signal is emited.
        Be aware of deadlock. Do not directly call any Neslab class methods
        in response to this signal.
    \fn Neslab::gotResponse()
        \brief Emited when got response from bath.
    \fn Neslab::connected()
        \brief Emited when the connection is established after
        Neslab::connectNeslab() slot is called.
    \fn Neslab::portOpeningFailed()
        \brief Emited when the call of Neslab::serialPort->open() method in the
        Neslab::connectNeslab() method failed.
    \fn Neslab::disconnected()
        \brief Emited when the connection was aborted by user action.
        \sa Neslab::setComPortName()
    \fn Neslab::connectionFailed()
        \brief Emited when some command fails to by succesfully executed which
        involves an invalid bath response more than Neslab::initTrialCount
        times.
        \sa onReadyData()
    \fn Neslab::setOnOffFinished()
        \brief Emited after the set on/off command is finished except for after
        Neslab::turnOn() and Neslab::turnOff() methods.
        \sa Neslab::connectNeslab(), Neslab::setOnOffCommand(),
        Neslab::turnOn(), Neslab::turnOff()
    \fn Neslab::readStatusFinished(QString msg)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readStatusCommand() is called.
        \param msg QString containing bath status.
    \fn Neslab::statusUpdated()
        \brief Emited after the correct response from the bath is received after
        the Neslab::updateStatusCommand() is called.
    \fn Neslab::turnedOn()
        \brief Emited after the correct response from the bath is received after
        the Neslab::turnOn() is called.
    \fn Neslab::turnedOff()
        \brief Emited after the correct response from the bath is received after
        the Neslab::turnOff() is called.
    \fn Neslab::readAcknowledgeFinished(unsigned char v1, unsigned char v2)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readAcknowledgeCommand() is called.
        \param v1 1st part of version number
        \param v2 last part of version number
    \fn Neslab::setLowTemperetareLimitFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setLowTemperetareLimitCommand() is called.
    \fn Neslab::readLowTemperatureLimitFinished(double T, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readLowTemperatureLimitCommand() is called.
        \param T            received temperature
        \param enhancedPrec idicates if enhanced precision was used
        \sa Neslab::setOnOffCommand(), NeslabTraits::PowerOnOff,
        Neslab::enhancedBathPrecision_
    \fn Neslab::setHighTemperatureLimitFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setHighTemperetareLimitCommand() is called.
    \fn Neslab::readHighTemperatureLimitFinished(double T, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readHighTemperatureLimitCommand() is called.
        \param T            received temperature
        \param enhancedPrec idicates if enhanced precision was used
        \sa Neslab::setOnOffCommand(), NeslabTraits::PowerOnOff,
        Neslab::enhancedBathPrecision_
    \fn Neslab::setHeatProportionalBandFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setHeatProportionalBandCommand() is called.
    \fn Neslab::readHeatProportionalBandFinished(double P, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readHeatProportionalBandCommand() is called.
        \param P            received heat proportional band
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always false.
    \fn Neslab::setHeatIntegralFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setHeatIntegralCommand() is called.
    \fn Neslab::readHeatIntegralFinished(double I, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readHeatIntegralCommand() is called.
        \param I            received heat integral
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always true.
    \fn Neslab::setHeatDerivativeFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setHeatDerivativeCommand() is called.
    \fn Neslab::readHeatDerivativeFinished(double D, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readHeatDerivativeCommand() is called.
        \param D            received heat derivative
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always true.
    \fn Neslab::setCoolProportionalBandFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setCoolProportionalBandCommand() is called.
    \fn Neslab::readCoolProportionalBandFinished(double P, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readCoolProportionalBandCommand() is called.
        \param P            received cool proportional band
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always false.
    \fn Neslab::setCoolIntegralFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setCoolIntegralCommand() is called.
    \fn Neslab::readCoolIntegralFinished(double I, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readCoolIntegralCommand() is called.
        \param I            received cool integral
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always true.
    \fn Neslab::setCoolDerivativeFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setCoolDerivativeCommand() is called.
    \fn Neslab::readCoolDerivativeFinished(double D, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readCoolDerivativeCommand() is called.
        \param D            received cool derivative
        \param enhancedPrec idicates if enhanced precision was used, it might be
        always true.
    \fn Neslab::readSetpointFinished(double T, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readSetpointCommand() is called.
        \param T            received setpoint temperature
        \param enhancedPrec idicates if enhanced precision was used
        \sa Neslab::setOnOffCommand(), NeslabTraits::PowerOnOff,
        Neslab::enhancedBathPrecision_
    \fn Neslab::setSetpointFinished()
        \brief Emited after the correct response from the bath is received after
        the Neslab::setSetpointCommand() is called.
    \fn Neslab::readExternalSensorFinished(double T, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readTemperatureCommand() is called if the external sensor is
        enabled.
        \param T            received setpoint temperature
        \param enhancedPrec idicates if enhanced precision was used
        \sa Neslab::setOnOffCommand(), NeslabTraits::PowerOnOff,
        Neslab::enhancedBathPrecision_
    \fn Neslab::readInternalTemperatureFinished(double T, bool enhancedPrec)
        \brief Emited after the correct response from the bath is received after
        the Neslab::readTemperatureCommand() is called if the external sensor is
        disabled.
        \param T            received setpoint temperature
        \param enhancedPrec idicates if enhanced precision was used
        \sa Neslab::setOnOffCommand(), NeslabTraits::PowerOnOff,
        Neslab::enhancedBathPrecision_
    \fn Neslab::connectNeslabInThread()
        \brief Internal signal emited in connectNeslab() method to transfer the
        action to the Neslab object containing thread, because QSerialPort
        cannot be controlled from multiple threads. It is connected to the
        onConnectNeslabInThread() slot.
    \fn Neslab::sendToSerial(const unsigned char *buffer, int n)
        \brief Internal signal emited when the command is being sent to the
        serial port to transfer the action to the Neslab object containing
        thread, because QSerialPort cannot be controlled from multiple threads.
        It is connected to the onSendToSerial() slot.
        \param buffer containig binary command
        \param n containing the number of command bytes.
        \warning The \p buffer parameter have to be dynamically allocated on
        the heap and will be deleted in the onSendToSerial() slot.
    \fn Neslab::startCmdTimer()
        \brief Internal signal emited when the command is being sent to start
        the Neslab::cmdTimer, which controls, if the waiting for the %Neslab
        bath response is not too long. It transfers the timer starting to the
        Neslab object containing thread because the QTimer object cannot be
        started from another thread. It is connected to the onStartCmdTimer()
        slot.
*/

/*!
    \brief This method sends commands to the bath restoring the default PIDs
    parameters.

    \sa defaultHeatP, defaultHeatI, defaultHeatD, defaultCoolP, defaultCoolI,
    defaultCoolD setHeatProportionalBandCommand(), setHeatIntegralCommand(),
    setHeatDerivativeCommand(), setCoolProportionalBandCommand(),
    setCoolIntegralCommand(), setCoolDerivativeCommand()
*/
void Neslab::restoreDefaultSettingsCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    locker.unlock();
    setHeatProportionalBandCommand(defaultHeatP);
    setHeatIntegralCommand(defaultHeatI);
    setHeatDerivativeCommand(defaultHeatD);
    setCoolProportionalBandCommand(defaultCoolP);
    setCoolIntegralCommand(defaultCoolI);
    setCoolDerivativeCommand(defaultCoolD);
}

/*!
   \brief This method opens the SerialPort connection and checks the %Neslab
   bath unit connection.

   This command must be called before any command could be send to the bath. It
   establishes serial port connection and checks if the serial port correctly
   responses to the sendSetOnOffCommand(). It uses setOnOffCommand() and
   default constructed PowerOnOffParm, which sets all its bytes to 2, which
   means do not change any value. The response from the bath sets the bytes to
   the values corresponding to the current bath state. For this purpose, it
   sets Neslab::connecting_ private variable to true notifying the onSetOnOff()
   method, which is invoked after the response to the sendSetOnOffCommand() is
   received, that this response belongs to connectNeslab() command and is
   intended as check of the connection. The onSetOnOff() method emits
   connected() and setOnOffFinished() signals after the correct response is
   received.

   Because QSerialPort class cannot be controlled from mlutiple threads, it is
   necessary to transfer the action to the Neslab object containing thread. It
   is done by emiting connectNeslabInThread() signal which is connected to
   onConnectNeslabInThread() private slot. All the work is done in this slot.

   /sa Neslab::serialPort, Neslab::wOnOffParm_
*/
void Neslab::connectNeslab()
{
    QMutexLocker locker(mutex);
    if (!connected_) {
        locker.unlock();
        emit connectNeslabInThread();
    } else {
        locker.unlock();
        emit connected();
    }
}

/*!
    \brief This command can be used to set unit parameters including turning
    on/off.
    \param parm Data bytes for the command.

    See The NeslabTraits::PowerOnOffParm for the details. The signals
    and setOnOffFinished() are emited from the onSetOnOff() method after the
    correct response is received.
    \sa connectNeslab(), turnOn(), turnOff(), isTurnedOn(),
    sendSetOnOffCommand(), onSetOnOff()
*/
void Neslab::setOnOffCommand(const PowerOnOffParm & parm)
{
    QMutexLocker locker(mutex);
    if (!connected_ && !connecting_) {
        return;
    }
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        wOnOffParm_.erase();
        wOnOffParm_ = parm;
        commandBuffer[static_cast<int>(Command::SetOnOff)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetOnOffCommand();
    } else {
        for (int ii = 0; ii < 8; ++ii ) {
            if (parm[ii] != static_cast<unsigned char>(2))
                wOnOffParm_[ii] = parm[ii];
        }
        commandBuffer[static_cast<int>(Command::SetOnOff)] = true;
    }
}

/*!
   \brief Turns the unit on.

   It uses sendSetOnOffCommand(). It sets Neslab::switchingOn_ private variable
   to true and Neslab::switchingOff_ to false to indicate, that the unit is
   turning on in the onSetOnOff() method is invoked after the response from the
   bath is received. The onSetOnOff() method also emits turnedOn() and
   setOnOffFinished() signals after the correct response is received.

   \sa turnOff()
 */
void Neslab::turnOn()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (statusUpdatedAfterOnOffCmd() && (rOnOffParm_[PowerOnOff::d1_unitOnOff] == 1)) {
        locker.unlock();
        emit turnedOn();
    } else {
        switchingOn_ = true;
        switchingOff_ = false;

        if (lastCommand == Command::None && !waitingForNextCommand_) {
            wOnOffParm_.erase();
            wOnOffParm_[0] = 1;
            commandBuffer[static_cast<int>(Command::SetOnOff)] = false;
            trialCount = initTrialCount;
            emit sendingCommand();
            sendSetOnOffCommand();
        } else {
            wOnOffParm_[0] = 1;
            commandBuffer[static_cast<int>(Command::SetOnOff)] = true;
        }
    }
}

/*!
   \brief Turns the unit off.

   It uses sendSetOnOffCommand(). It sets Neslab::switchingOn_ private variable
   to false and Neslab::switchingOff_ to true to indicate, that the unit is
   turning on in the onSetOnOff() method is invoked after the response from the
   bath is received. The onSetOnOff() method also emits turnedOff() and
   setOnOffFinished() signals after the correct response is received.

   \sa turnOn()
 */
void Neslab::turnOff()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (statusUpdatedAfterOnOffCmd() && (rOnOffParm_[PowerOnOff::d1_unitOnOff] == 0)) {
        locker.unlock();
        emit turnedOff();
    } else {
        switchingOn_ = false;
        switchingOff_ = true;

        if (lastCommand == Command::None && !waitingForNextCommand_) {
            wOnOffParm_.erase();
            wOnOffParm_[0] = 0;
            commandBuffer[static_cast<int>(Command::SetOnOff)] = false;
            trialCount = initTrialCount;
            emit sendingCommand();
            sendSetOnOffCommand();
        } else {
            wOnOffParm_[0] = 0;
            commandBuffer[static_cast<int>(Command::SetOnOff)] = true;
        }
    }
}

/*!
   \brief Sends request to the bath to get the bath parameters including if the
   bath is turned on.

   It uses setOnOffCommand() and default constructed
   NeslabTraits::PowerOnOffParm, which sets all its bytes to 2, which means do
   not change any value. The response from the bath sets the bytes to the
   values corresponding to the current bath state. After the response of the
   bath is received, the method onSetOnOff() emits setOnOffFinished() signal.
 */
void Neslab::isTurnedOn()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    locker.unlock();
    PowerOnOffParm powerOnOffParm;
    setOnOffCommand(powerOnOffParm);
}

/*!
   \brief Requests status from the bath. The response returns status message.

   The Neslab::readStatusSilently_ variable is set to false, indicating that the
   onReadStatus() method, which is invoked after the response from the bath is
   received, emits readStatusFinished(QString msg) signal at the correct
   answer.

   \sa updateStatusCommand(), sendReadStatusCommand()
*/
void Neslab::readStatusCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    readStatusSilently_ = false;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadStatus)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadStatusCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadStatus)] = true;
    }
}

/*!
   \brief Requests status from the bath. The response does not return status
   message.

   The Neslab::readStatusSilently_ variable is set to true, indicating that the
   onReadStatus() method, which is invoked after the response from the bath is
   received, emits statusUpdated() signal at the correct answer.

   \sa readStatusCommand(), sendReadStatusCommand()
 */
void Neslab::updateStatusCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (!commandBuffer[static_cast<int>(Command::ReadStatus)])
        readStatusSilently_ = true;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadStatus)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadStatusCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadStatus)] = true;
    }
}

/*!
   \brief Requests the protocol version from the bath.

   The onReadAcknowledge() method, which is invoked after the response from the
   bath is received, emits readAcknowledgeFinished() signal at the correct
   answer.

   \sa sendReadAcknowledgeCommand()
 */
void Neslab::readAcknowledgeCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadAcknowledge)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadAcknowledgeCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadAcknowledge)] = true;
    }
}

/*!
   \brief Requests setting the low temperature limit of the bath.
   \param T requestend low temperature limit in °C

   The onSetLowTemperatureLimit() method, which is invoked after the response
   from the bath is received, emits setLowTemperetareLimitFinished() signal at
   the correct answer.

   \sa readLowTemperatureLimitCommand(), sendSetLowTemperatureLimitCommand()
 */
void Neslab::setLowTemperatureLimitCommand(double T)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wLowTLim_ = T;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetLowTemperatureLimit)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetLowTemperatureLimitCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetLowTemperatureLimit)] = true;
    }
}

/*!
   \brief Requests the low temperature limit value from the bath.

   The onReadLowTemperatureLimit() method, which is invoked after the response
   from the bath is received, emits readLowTemperatureLimitFinished() signal at
   the correct answer.

   \sa setLowTemperatureLimitCommand(), sendReadLowTemperatureLimitCommand()
 */
void Neslab::readLowTemperatureLimitCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadLowTemperatureLimit)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadLowTemperatureLimitCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadLowTemperatureLimit)] = true;
    }
}

/*!
   \brief Requests setting the high temperature limit of the bath.
   \param T requestend high temperature limit in °C

   The onSetHighTemperatureLimit() method, which is invoked after the response
   from the bath is received, emits setHighTemperatureLimitFinished() signal at
   the correct answer.

   \sa readHighTemperatureLimitCommand(), sendSetHighTemperatureLimitCommand()
 */
void Neslab::setHighTemperatureLimitCommand(double T)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wHighTLim_ = T;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetHighTemperatureLimit)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetHighTemperatureLimitCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetHighTemperatureLimit)] = true;
    }
}

/*!
   \brief Requests the high temperature limit value from the bath.

   The onReadHighTemperatureLimit() method, which is invoked after the response
   from the bath is received, emits readHighTemperatureLimitFinished() signal at
   the correct answer.

   \sa setHighTemperatureLimitCommand(), sendReadHighTemperatureLimitCommand()
 */
void Neslab::readHighTemperatureLimitCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadHighTemperatureLimit)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadHighTemperatureLimitCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadHighTemperatureLimit)] = true;
    }
}

/*!
   \brief Requests setting the heat proportional band of the bath heat PID unit.
   \param P requested heat proportional band.

   The onSetHeatProportionalBand() method, which is invoked after the response
   from the bath is received, emits setHeatProportionalBandFinished() signal at
   the correct answer.

   The default value of the heat proportional band can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readHeatProportionalBandCommand(), sendSetHeatProportionalBandCommand(),
   Neslab::defaultHeatP
 */
void Neslab::setHeatProportionalBandCommand(double P)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wHeatP_ = P;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetHeatProportionalBand)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetHeatProportionalBandCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetHeatProportionalBand)] = true;
    }
}

/*!
   \brief Requests the heat proportional band value from the bath heat PID unit.

   The onReadHeatProportionalBand() method, which is invoked after the response
   from the bath is received, emits readHeatProportionalBandFinished() signal at
   the correct answer.

   \sa setHeatProportionalBandCommand(), sendReadHeatProportionalBandCommand()
 */
void Neslab::readHeatProportionalBandCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadHeatProportionalBand)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadHeatProportionalBandCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadHeatProportionalBand)] = true;
    }
}

/*!
   \brief Requests setting the heat integral of the bath heat PID unit.
   \param I requested heat integral.

   The onSetHeatIntegral() method, which is invoked after the response from the
   bath is received, emits setHeatIntegralFinished() signal at the correct
   answer.

   The default value of the heat integral can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readHeatIntegralCommand(), sendSetHeatIntegralCommand(),
   Neslab::defaultHeatI
 */
void Neslab::setHeatIntegralCommand(double I)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wHeatI_ = I;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetHeatIntegral)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetHeatIntegralCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetHeatIntegral)] = true;
    }
}

/*!
   \brief Requests the heat integral value from the bath heat PID unit.

   The onReadHeatIntegral() method, which is invoked after the response from
   the bath is received, emits readHeatIntegralFinished() signal at the correct
   answer.

   \sa setHeatIntegralCommand(), sendReadHeatIntegralCommand()
 */
void Neslab::readHeatIntegralCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadHeatIntegral)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadHeatIntegralCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadHeatIntegral)] = true;
    }
}

/*!
   \brief Requests setting the heat derivative of the bath heat PID unit.
   \param D requested heat derivative.

   The onSetHeatDerivative() method, which is invoked after the response from
   the bath is received, emits setHeatDerivativeFinished() signal at the
   correct answer.

   The default value of the heat integral can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readHeatDerivativeCommand(), sendSetHeatDerivativeCommand(),
   Neslab::defaultHeatD
 */
void Neslab::setHeatDerivativeCommand(double D)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wHeatD_ = D;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetHeatDerivative)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetHeatDerivativeCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetHeatDerivative)] = true;
    }
}

/*!
   \brief Requests the heat derivative value from the bath heat PID unit.

   The onReadHeatDerivative() method, which is invoked after the response from
   the bath is received, emits readHeatDerivativeFinished() signal at the
   correct answer.

   \sa setHeatDerivativeCommand(), sendReadHeatDerivativeCommand()
 */
void Neslab::readHeatDerivativeCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadHeatDerivative)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadHeatDerivativeCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadHeatDerivative)] = true;
    }
}

/*!
   \brief Requests setting the cool proportional band of the bath cool PID unit.
   \param P requested cool proportional band.

   The onSetCoolProportionalBand() method, which is invoked after the response
   from the bath is received, emits setCoolProportionalBandFinished() signal at
   the correct answer.

   The default value of the heat integral can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readCoolProportionalBandCommand(), sendSetCoolProportionalBandCommand(),
   Neslab::defaultCoolP
 */
void Neslab::setCoolProportionalBandCommand(double P)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wCoolP_ = P;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetCoolProportionalBand)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetCoolProportionalBandCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetCoolProportionalBand)] = true;
    }
}

/*!
   \brief Requests the cool proportional band value from the bath cool PID unit.

   The onReadCoolProportionalBand() method, which is invoked after the response
   from the bath is received, emits readCoolProportionalBandFinished() signal at
   the correct answer.

   \sa setCoolProportionalBandCommand(), sendReadCoolProportionalBandCommand()
 */
void Neslab::readCoolProportionalBandCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadCoolProportionalBand)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadCoolProportionalBandCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadCoolProportionalBand)] = true;
    }
}

/*!
   \brief Requests setting the heat integral of the bath cool PID unit.
   \param I requested cool integral.

   The onSetCoolIntegral() method, which is invoked after the response from the
   bath is received, emits setCoolIntegralFinished() signal at the correct
   answer.

   The default value of the heat integral can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readCoolIntegralCommand(), sendSetCoolIntegralCommand(),
   Neslab::defaultCoolI
 */
void Neslab::setCoolIntegralCommand(double I)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wCoolI_ = I;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetCoolIntegral)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetCoolIntegralCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetCoolIntegral)] = true;
    }
}

/*!
   \brief Requests the cool integral value from the bath cool PID unit.

   The onReadCoolIntegral() method, which is invoked after the response from
   the bath is received, emits readCoolIntegralFinished() signal at the correct
   answer.

   \sa setCoolIntegralCommand(), sendReadCoolIntegralCommand()
 */
void Neslab::readCoolIntegralCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadCoolIntegral)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadCoolIntegralCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadCoolIntegral)] = true;
    }
}

/*!
   \brief Requests setting the cool derivative of the bath cool PID unit.
   \param D requested cool derivative.

   The onSetCoolDerivative() method, which is invoked after the response from
   the bath is received, emits setCoolDerivativeFinished() signal at the
   correct answer.

   The default value of the heat integral can be set by the
   restoreDefaultSettingsCommand() method.

   \sa readCoolDerivativeCommand(), sendSetCoolDerivativeCommand(),
   Neslab::defaultCoolD
 */
void Neslab::setCoolDerivativeCommand(double D)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wCoolD_ = D;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::SetCoolDerivative)] = false;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendSetCoolDerivativeCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetCoolDerivative)] = true;
    }
}

/*!
   \brief Requests the cool derivative value from the bath cool PID unit.

   The onReadCoolDerivative() method, which is invoked after the response from
   the bath is received, emits readCoolDerivativeFinished() signal at the
   correct answer.

   \sa setCoolDerivativeCommand(), sendReadCoolDerivativeCommand()
 */
void Neslab::readCoolDerivativeCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadCoolDerivative)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadCoolDerivativeCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadCoolDerivative)] = true;
    }
}

/*!
   \brief Requests the setpoint value from the bath.

   The setpoint is the same for the external and internal sensor. The sensor in
   use can be set by setOnOffCommand() (see NeslabTraits::PowerOnOff). The
   onReadSetpoint() method, which is invoked after the response from the bath
   is received, emits readSetpointFinished() signal at the correct answer.

   \sa setSetpointCommand(), sendReadSetpointCommand()
 */
void Neslab::readSetpointCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        commandBuffer[static_cast<int>(Command::ReadSetpoint)] = true;
        trialCount = initTrialCount;
        emit sendingCommand();
        sendReadSetpointCommand();
    } else {
        commandBuffer[static_cast<int>(Command::ReadSetpoint)] = true;
    }
}

/*!
   \brief Requests setting the setpoint of the bath.
   \param T requestend setpoint in °C

   The setpoint is the same for the external and internal sensor. The sensor in
   use can be set by setOnOffCommand() (see NeslabTraits::PowerOnOff). The
   onSetSetpoint() method, which is invoked after the response from the bath is
   received, emits setSetpointFinished() signal at the correct answer.

   \sa readSetpointCommand(), readTemperatureCommand(), sendSetSetpointCommand()
 */
void Neslab::setSetpointCommand(double T)
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    wSetpointT_ = T;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        trialCount = initTrialCount;
        commandBuffer[static_cast<int>(Command::SetSetpoint)] = false;
        emit sendingCommand();
        sendSetSetpointCommand();
    } else {
        commandBuffer[static_cast<int>(Command::SetSetpoint)] = true;
    }
}

/*!
   \brief Requests the temperature value from the bath.

   It chooses between the external and internal sensor according to value store
   in Neslab::rOnOffParm_, which was received from the response after the last
   sendPowerOnOffCommand() command was sent. The sensor in use can be set by
   setOnOffCommand() (see NeslabTraits::PowerOnOff). The
   onReadInternalTemperature() or onReadExternalSensor() method, which is
   invoked after the response from the bath is received, emits
   readInternalTemperatureFinished() or readExternalSensorFinished() signal at
   the correct answer.

   \sa setSetpointCommand(), sendReadInternalTemperatureCommand(),
   sendReadExternalSensorCommand()
 */
void Neslab::readTemperatureCommand()
{
    QMutexLocker locker(mutex);
    if (!connected_)
        return;
    if (lastCommand == Command::None && !waitingForNextCommand_) {
        trialCount = initTrialCount;
        if (rOnOffParm_[PowerOnOff::d2_sensorEnabled] == 1) {
            commandBuffer[static_cast<int>(Command::ReadExternalSensor)] = true;
            emit sendingCommand();
            sendReadExternalSensorCommand();
        } else {
            commandBuffer[static_cast<int>(Command::ReadInternalTemperature)] = true;
            emit sendingCommand();
            sendReadInternalTemperatureCommand();
        }
    } else {
        if (rOnOffParm_[PowerOnOff::d2_sensorEnabled] == 1) {
            commandBuffer[static_cast<int>(Command::ReadExternalSensor)] = true;
        } else {
            commandBuffer[static_cast<int>(Command::ReadInternalTemperature)] = true;
        }
    }
}

/*!
    \brief This method is invoked after the data from the serial port is
    received, checks wheather the data is valid and calls appropriate function
    for the response processing.

    It chooses according to the value stored in Neslab::lastCommand, how should
    the response look like and validates the response calling the
    validateResponse() method. Then it chooses processing method corresponding
    to the response to the last command requested. At last, it starts the
    Neslab::waitingTimer, which manages delay between two consecutive commands,
    setting the Neslab::waitingForNextCommand_ to true, Neslab::lastCommand to
    NeslabTraits::Command::None and reseting Neslab::trialCount to
    Neslab::initTrialCount

    If the response is not valid, it decreases the Neslab::trialCount and
    checks, wheather it is greater than zero. If so, it resends the last
    command by starting the Neslab::waitingTimer and setting
    Neslab::waitingForNextCommand_ to true. If the number of unsuccesfull
    trials to send the command reaches the Neslab::initTrialCount, it sets
    Neslab::connected_ to false and emits connectionFailed() signal.
*/
void Neslab::onReadyData()
{
    // getting rid of unexpected bath response
    if (lastCommand == NeslabTraits::Command::None) {
        serialPort->readAll();
        return;
    }
    // stop the timer which controls if the waiting for bath response is not
    // to long
    cmdTimer->stop();

    // converting the data to unsigned char
    QByteArray data = serialPort->readAll();
    int n = data.size();
    unsigned char *buffer = reinterpret_cast<unsigned char*>(data.data());

    // response validation
    bool respValid = validateResponse(buffer, n, lastCommand);
    bool ok = false; // indicates the valid answer from the command specific
    // processing methods

    if (respValid) {

        // choosing which command specific processing method is invoked
        // according to the last called command
        switch (lastCommand) {
        case Command::SetOnOff :
            ok = onSetOnOff(buffer, n);
            break;
        case Command::ReadStatus :
            ok = onReadStatus(buffer, n);
            break;
        case Command::ReadAcknowledge :
            ok = onReadAcknowledge(buffer, n);
            break;
        case Command::SetLowTemperatureLimit :
            ok = onSetLowTemperatureLimit(buffer, n);
            break;
        case Command::ReadLowTemperatureLimit :
            ok = onReadLowTemperatureLimit(buffer, n);
            break;
        case Command::SetHighTemperatureLimit :
            ok = onSetHighTemperatureLimit(buffer, n);
            break;
        case Command::ReadHighTemperatureLimit :
            ok = onReadHighTemperatureLimit(buffer, n);
            break;
        case Command::SetHeatProportionalBand :
            ok = onSetHeatProportionalBand(buffer, n);
            break;
        case Command::ReadHeatProportionalBand :
            ok = onReadHeatProportionalBand(buffer, n);
            break;
        case Command::SetHeatIntegral :
            ok = onSetHeatIntegral(buffer, n);
            break;
        case Command::ReadHeatIntegral :
            ok = onReadHeatIntegral(buffer, n);
            break;
        case Command::SetHeatDerivative :
            ok = onSetHeatDerivative(buffer, n);
            break;
        case Command::ReadHeatDerivative :
            ok = onReadHeatDerivative(buffer, n);
            break;
        case Command::SetCoolProportionalBand :
            ok = onSetCoolProportionalBand(buffer, n);
            break;
        case Command::ReadCoolProportionalBand :
            ok = onReadCoolProportionalBand(buffer, n);
            break;
        case Command::SetCoolIntegral :
            ok = onSetCoolIntegral(buffer, n);
            break;
        case Command::ReadCoolIntegral :
            ok = onReadCoolIntegral(buffer, n);
            break;
        case Command::SetCoolDerivative :
            ok = onSetCoolDerivative(buffer, n);
            break;
        case Command::ReadCoolDerivative :
            ok = onReadCoolDerivative(buffer, n);
            break;
        case Command::ReadSetpoint :
            ok = onReadSetpoint(buffer, n);
            break;
        case Command::SetSetpoint :
            ok = onSetSetpoint(buffer, n);
            break;
        case Command::ReadExternalSensor :
            ok = onReadExternalSensor(buffer, n);
            break;
        case Command::ReadInternalTemperature :
            ok = onReadInternalTemperature(buffer, n);
            break;
        default :
            break;
        }

        // if response is valid, sending new command
        if (ok) {
            waitingForNextCommand_ = true;
            trialCount = initTrialCount;
            waitingTimer->setSingleShot(true);
            waitingTimer->start(cmdDelay);
            lastCommand = Command::None;
        }
    }

    // if response is not valid, resending command or disconnecting, if
    // the initTrialCount number of unsuccesful trials was reached
    if (!respValid || !ok) {
        if ((--trialCount)) {
            waitingForNextCommand_ = true;
            waitingTimer->setSingleShot(true);
            waitingTimer->start(cmdDelay);
        } else {
            connected_ = false;
            emit connectionFailed();
        }
    }
}

/*!
    \brief This slot is trigered when the Neslab::cmdDelay time exceeds by the
    timeout of the Neslab::waitingTimer, setting Neslab::waitingForNextCommand_
    to false and sending next command by the sendCommand() method.
    \sa onReadyData()
 */
void Neslab::onWaitingFinished()
{
    waitingForNextCommand_ = false;
    sendCommand();
}

/*!
    \brief This function is executed when the Neslab::waitForResponse time
    exceeds.

    It is triggered by the Neslab::cmdTimer timer, which is set to single shot
    after Neslab::waitForResponse ms in onStartCmdTimer method. It
    decreases the Neslab::trialCount variable and checks if the number of
    trials to send the command did not reach the Neslab::initTrialCount value.
    If not, it resends the command by calling the sendCommand() method, else,
    it emits connectionFailed() signal.
 */
void Neslab::onNoResponse()
{
    if ((--trialCount)) {
        sendCommand();
    }
    else {
        connected_ = false;
        lastCommand = Command::None;
        emit connectionFailed();
    }
}

/*!
   \brief Starts Neslab::cmdTimer, which is used to control if the waiting for
   a response on a command is not too long.
 */
void Neslab::onStartCmdTimer()
{
    cmdTimer->setSingleShot(true);
    cmdTimer->start(waitForResponse);
}

/*!
   \brief This private slot is executed after connectNeslabInThread() signal is
   emited from the connectNeslab() public slot to transfer the action to the
   Neslab object containing thread, because QSerialPort cannot be controlled
   from multiple threads. For details se the description of connectNeslab()
   slot.
 */
void Neslab::onConnectNeslabInThread()
{
    if (!serialPort->isOpen()) {
#ifdef VIRTUALSERIALPORT
        serialPort->setPortName(comPortName_);
        serialPort->setBaudRate(VirtualSerialPort::Baud9600);
        serialPort->setDataBits(VirtualSerialPort::Data8);
        serialPort->setParity(VirtualSerialPort::NoParity);
        serialPort->setStopBits(VirtualSerialPort::OneStop);
        serialPort->setFlowControl(VirtualSerialPort::NoFlowControl);
        if (!serialPort->open(VirtualSerialPort::ReadWrite)) {
            emit portOpeningFailed();
            return;
        }
#else // VIRTUALSERIALPORT
        serialPort->setPortName(comPortName_);
        serialPort->setBaudRate(QSerialPort::Baud9600);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if (!serialPort->open(QIODevice::ReadWrite)) {
            emit portOpeningFailed();
            return;
        }
#endif // VIRTUALSERIALPORT
    }
    connecting_ = true;
    PowerOnOffParm parm;
    setOnOffCommand(parm);
}

/*!
   \brief Neslab::onSendToSerial
   \param buffer
   \param n
 */
void Neslab::onSendToSerial(const unsigned char *buffer, int n)
{
#ifdef VIRTUALSERIALPORT
    if (!serialPort->isOpen())
        serialPort->open(VirtualSerialPort::ReadWrite);
#else // VIRTUALSERIALPORT
    if (!serialPort->isOpen())
        serialPort->open(QIODevice::ReadWrite);
#endif // VIRTUALSERIALPORT
    serialPort->write(reinterpret_cast<char*>(const_cast<unsigned char*>(buffer)), n);
    delete[] buffer;
}

/*!
   \brief Indicates wheather the on/off command is running
   (sendSetOnOffCommand()).
   \return true if the bath got response after on/off command.
   \sa statusUpdatedAfterOnOffCmd_
*/
bool Neslab::statusUpdatedAfterOnOffCmd() const
{
    return statusUpdatedAfterOnOffCmd_;
}

/*!
    \brief Neslab::sendCommand
*/
void Neslab::sendCommand()
{
    QString strCmdBuff;
    for (int ii = 0; ii < static_cast<int>(Command::None); ++ii)
        strCmdBuff.append(QString("%1 ").arg(commandBuffer[static_cast<int>(ii)]));
    Command cmd = static_cast<Command>(0);
    if (lastCommand == Command::None) {
        trialCount = initTrialCount;
        while (cmd != Command::None) {
            if (commandBuffer[static_cast<int>(cmd)]) {
                // next command found
                break;
            }
            ++cmd;
        }
    } else {
        // repeat last command
        cmd = lastCommand;
    }
    switch (cmd) {
    case Command::SetOnOff :
        commandBuffer[static_cast<int>(Command::SetOnOff)] = false;
        sendSetOnOffCommand();
        break;
    case Command::ReadStatus :
        sendReadStatusCommand();
        break;
    case Command::ReadAcknowledge :
        sendReadAcknowledgeCommand();
        break;
    case Command::SetLowTemperatureLimit :
        commandBuffer[static_cast<int>(Command::SetLowTemperatureLimit)] = false;
        sendSetLowTemperatureLimitCommand();
        break;
    case Command::ReadLowTemperatureLimit :
        sendReadLowTemperatureLimitCommand();
        break;
    case Command::SetHighTemperatureLimit :
        commandBuffer[static_cast<int>(Command::SetHighTemperatureLimit)] = false;
        sendSetHighTemperatureLimitCommand();
        break;
    case Command::ReadHighTemperatureLimit :
        sendReadHighTemperatureLimitCommand();
        break;
    case Command::SetHeatProportionalBand :
        commandBuffer[static_cast<int>(Command::SetHeatProportionalBand)] = false;
        sendSetHeatProportionalBandCommand();
        break;
    case Command::ReadHeatProportionalBand :
        sendReadHeatProportionalBandCommand();
        break;
    case Command::SetHeatIntegral :
        commandBuffer[static_cast<int>(Command::SetHeatIntegral)] = false;
        sendSetHeatIntegralCommand();
        break;
    case Command::ReadHeatIntegral :
        sendReadHeatIntegralCommand();
        break;
    case Command::SetHeatDerivative :
        commandBuffer[static_cast<int>(Command::SetHeatDerivative)] = false;
        sendSetHeatDerivativeCommand();
        break;
    case Command::ReadHeatDerivative :
        sendReadHeatDerivativeCommand();
        break;
    case Command::SetCoolProportionalBand :
        commandBuffer[static_cast<int>(Command::SetCoolProportionalBand)] = false;
        sendSetCoolProportionalBandCommand();
        break;
    case Command::ReadCoolProportionalBand :
        sendReadCoolProportionalBandCommand();
        break;
    case Command::SetCoolIntegral :
        commandBuffer[static_cast<int>(Command::SetCoolIntegral)] = false;
        sendSetCoolIntegralCommand();
        break;
    case Command::ReadCoolIntegral :
        sendReadCoolIntegralCommand();
        break;
    case Command::SetCoolDerivative :
        commandBuffer[static_cast<int>(Command::SetCoolDerivative)] = false;
        sendSetCoolDerivativeCommand();
        break;
    case Command::ReadCoolDerivative :
        sendReadCoolDerivativeCommand();
        break;
    case Command::ReadSetpoint :
        sendReadSetpointCommand();
        break;
    case Command::SetSetpoint :
        commandBuffer[static_cast<int>(Command::SetSetpoint)] = false;
        sendSetSetpointCommand();
        break;
    case Command::ReadExternalSensor :
        sendReadExternalSensorCommand();
        break;
    case Command::ReadInternalTemperature :
        sendReadInternalTemperatureCommand();
        break;
    default :
        // no command in the buffer
        emit gotResponse();
        break;
    }
}

/*!
   \brief Neslab::sendSetOnOffCommand
 */
void Neslab::sendSetOnOffCommand()
{
    statusUpdatedAfterOnOffCmd_ = false;
    int n = 14; // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetOnOff)][ii];
    cmd[4] = 8;
    for (ii = 0; ii < 8; ++ii)
        cmd[5 + ii] = wOnOffParm_[ii];
    cmd[13] = Neslab::checkSum(cmd, 13);
    lastCommand = Command::SetOnOff;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadStatusCommand
 */
void Neslab::sendReadStatusCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadStatus)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadStatus;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadAcknowledgeCommand
 */
void Neslab::sendReadAcknowledgeCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadAcknowledge)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadAcknowledge;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetLowTemperatureLimitCommand
 */
void Neslab::sendSetLowTemperatureLimitCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetLowTemperatureLimit)][ii];
    cmd[4] = 2;

    int iT;
    if (enhancedBathPrecision_)
        iT = static_cast<int>(wLowTLim_ * 100 + 0.5);
    else
        iT = static_cast<int>(wLowTLim_ * 10 + 0.5);
    if (iT < 0) {
        iT = maxNumber - 1 + iT;
    }
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetLowTemperatureLimit;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadLowTemperatureLimitCommand
 */
void Neslab::sendReadLowTemperatureLimitCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadLowTemperatureLimit)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadLowTemperatureLimit;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetHighTemperatureLimitCommand
 */
void Neslab::sendSetHighTemperatureLimitCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetHighTemperatureLimit)][ii];
    cmd[4] = 2;

    int iT;
    if (enhancedBathPrecision_)
        iT = static_cast<int>(wHighTLim_ * 100 + 0.5);
    else
        iT = static_cast<int>(wHighTLim_ * 10 + 0.5);
    if (iT < 0)
        iT = maxNumber - 1 + iT;
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetHighTemperatureLimit;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadHighTemperatureLimitCommand
 */
void Neslab::sendReadHighTemperatureLimitCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadHighTemperatureLimit)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadHighTemperatureLimit;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetHeatProportionalBandCommand
 */
void Neslab::sendSetHeatProportionalBandCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetHeatProportionalBand)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wHeatP_ * 10 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetHeatProportionalBand;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadHeatProportionalBandCommand
 */
void Neslab::sendReadHeatProportionalBandCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadHeatProportionalBand)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadHeatProportionalBand;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetHeatIntegralCommand
 */
void Neslab::sendSetHeatIntegralCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetHeatIntegral)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wHeatI_ * 100 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetHeatIntegral;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadHeatIntegralCommand
 */
void Neslab::sendReadHeatIntegralCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadHeatIntegral)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadHeatIntegral;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetHeatDerivativeCommand
 */
void Neslab::sendSetHeatDerivativeCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetHeatDerivative)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wHeatD_ * 100 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetHeatDerivative;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadHeatDerivativeCommand
 */
void Neslab::sendReadHeatDerivativeCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadHeatDerivative)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadHeatDerivative;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetCoolProportionalBandCommand
 */
void Neslab::sendSetCoolProportionalBandCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetCoolProportionalBand)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wCoolP_ * 10 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetCoolProportionalBand;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadCoolProportionalBandCommand
 */
void Neslab::sendReadCoolProportionalBandCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadCoolProportionalBand)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadCoolProportionalBand;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetCoolIntegralCommand
 */
void Neslab::sendSetCoolIntegralCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetCoolIntegral)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wCoolI_ * 100 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetCoolIntegral;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadCoolIntegralCommand
 */
void Neslab::sendReadCoolIntegralCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadCoolIntegral)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadCoolIntegral;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetCoolDerivativeCommand
 */
void Neslab::sendSetCoolDerivativeCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetCoolDerivative)][ii];
    cmd[4] = 2;

    int iT = static_cast<int>(wCoolD_ * 100 + 0.5);
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetCoolDerivative;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadCoolDerivativeCommand
 */
void Neslab::sendReadCoolDerivativeCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadCoolDerivative)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadCoolDerivative;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadSetpointCommand
 */
void Neslab::sendReadSetpointCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(NeslabTraits::Command::ReadSetpoint)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadSetpoint;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendSetSetpointCommand
 */
void Neslab::sendSetSetpointCommand()
{
    int n = 8;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(NeslabTraits::Command::SetSetpoint)][ii];
    cmd[4] = 2;

    int iT;
    if (enhancedBathPrecision_)
        iT = static_cast<int>(wSetpointT_ * 100 + 0.5);
    else
        iT = static_cast<int>(wSetpointT_ * 10 + 0.5);
    if (iT < 0)
        iT = maxNumber - 1 + iT;
    cmd[5] = iT / 256;
    cmd[6] = iT % 256;
    cmd[7] = Neslab::checkSum(cmd, 7);
    lastCommand = Command::SetSetpoint;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadExternalSensorCommand
 */
void Neslab::sendReadExternalSensorCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(NeslabTraits::Command::ReadExternalSensor)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadExternalSensor;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendReadInternalTemperatureCommand
 */
void Neslab::sendReadInternalTemperatureCommand()
{
    int n = 6;
    unsigned char * cmd = new unsigned char[n];
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(NeslabTraits::Command::ReadInternalTemperature)][ii];
    cmd[4] = 0;
    cmd[5] = Neslab::checkSum(cmd, 5);
    lastCommand = Command::ReadInternalTemperature;
    emit startCmdTimer();
    emit sendToSerial(cmd, n);
}

/*!
   \brief Neslab::sendToSerial
   \param cmd
 */
void Neslab::strSendToSerial(const QString &cmd)
{
    unsigned char *buffer;
    int n;
    hexToByte(&buffer, &n, cmd);
    emit sendToSerial(buffer, n);
    delete[] buffer;
}

/*!
   \brief Neslab::onSetOnOff
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetOnOff(const unsigned char *buffer, int n)
{
    if (n >= 14) {
        if (static_cast<unsigned char>(8) != buffer[4])
            return false;

        for (int ii = 0; ii < 8; ++ii) {
            if (!(buffer[ii + 5] == 0 || buffer[ii + 5] == 1)) {
                return false;
            }
        }

        for (int ii = 0; ii < 8; ++ii)
            rOnOffParm_[ii] = buffer[ii + 5];

        enhancedBathPrecision_ = rOnOffParm_[PowerOnOff::d6_01precDegCEnable];

        if (!statusUpdatedAfterOnOffCmd()) {
            statusUpdatedAfterOnOffCmd_ = true;
        }
        if (connecting_) {
            connecting_ = false;
            connected_ = true;
            emit connected();
        }
        if (switchingOn_) {
            switchingOn_ = false;
            emit turnedOn();
        } else if (switchingOff_) {
            switchingOff_ = false;
            emit turnedOff();
        } else
            emit setOnOffFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadStatus
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadStatus(const unsigned char *buffer, int n)
{
    if (n >= 11) {
        bathStatus_.d1 = static_cast<BathStatusTraits::d1>(buffer[5]);
        bathStatus_.d2 = static_cast<BathStatusTraits::d2>(buffer[6]);
        bathStatus_.d3 = static_cast<BathStatusTraits::d3>(buffer[7]);
        bathStatus_.d4 = static_cast<BathStatusTraits::d4>(buffer[8]);
        bathStatus_.d5 = static_cast<BathStatusTraits::d5>(buffer[9]);

        QString boolTrue(tr("true"));
        QString boolFalse(tr("false"));
        commandBuffer[static_cast<int>(Command::ReadStatus)] = false;
        if (readStatusSilently_) {
            emit statusUpdated();
        } else {
            QString msg(tr("Bath status:\n"
                           "RTD1 Open Fault = %1\n"
                           "RTD1 Shorted Fault = %2\n"
                           "RTD1 Open = %3\n"
                           "RTD1 Shorted = %4\n"
                           "RTD3 Open Fault = %5\n"
                           "RTD3 Shorted Fault = %6\n"
                           "RTD3 Open = %7\n"
                           "RTD3 Shorted = %8\n\n"

                           "RTD2 Open Fault = %9\n"
                           "RTD2 Shorted Fault = %10\n"
                           "RTD2 Open Warn = %11\n"
                           "RTD2 Shorted Warn = %12\n"
                           "RTD2 Open = %13\n"
                           "RTD2 Shorted = %14\n"
                           "Refrig High Temp = %15\n"
                           "HTC Fault = %16\n\n"

                           "High Fixed Temp Fault = %17\n"
                           "Low Fixed Temp Fault = %18\n"
                           "High Temp Fault = %19\n"
                           "Low Temp Fault = %20\n"
                           "Low Level Fault = %21\n"
                           "High Temp Warn = %22\n"
                           "Low Temp Warn = %23\n"
                           "Low Level Warn = %24\n\n"

                           "Buzzer On = %25\n"
                           "Alarm Muted = %26\n"
                           "Unit Faulted = %27\n"
                           "Unit Stopping = %28\n"
                           "Unit On = %29\n"
                           "Pump On = %30\n"
                           "Compressor On = %31\n"
                           "Heater On = %32\n\n"

                           "RTD2 Controlling = %33\n"
                           "Heat LED Flashing = %34\n"
                           "Heat LED On = %35\n"
                           "Cool LED Flashing = %36\n"
                           "Cool LED On = %37")

                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD1OpenFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD1ShortedFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD1Open) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD1Shorted) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD3OpenFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD3ShortedFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD3Open) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d1 & BathStatusTraits::d1::RTD3Shorted) ?
                                 boolTrue : boolFalse)

                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2OpenFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2ShortedFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2OpenWarn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2ShortedWarn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2Open) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RTD2Shorted) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::RefrigHighTemp) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d2 & BathStatusTraits::d2::HTCFault) ?
                                 boolTrue : boolFalse)

                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::HighFixedTempFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::LowFixedTempFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::HighTempFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::LowTempFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::LowLevelFault) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::HighTempWarn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::LowTempWarn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d3 & BathStatusTraits::d3::LowLevelWarn) ?
                                 boolTrue : boolFalse)

                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::BuzzerOn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::AlarmMuted) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::UnitFaulted) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::UnitStopping) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::UnitOn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::PumpOn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::CompressorOn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d4 & BathStatusTraits::d4::HeaterOn) ?
                                 boolTrue : boolFalse)

                        .arg(static_cast<bool>(bathStatus_.d5 & BathStatusTraits::d5::RTD2Controlling) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d5 & BathStatusTraits::d5::HeatLEDFlashing) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d5 & BathStatusTraits::d5::HeatLEDOn) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d5 & BathStatusTraits::d5::CoolLEDFlashing) ?
                                 boolTrue : boolFalse)
                        .arg(static_cast<bool>(bathStatus_.d5 & BathStatusTraits::d5::CoolLEDOn) ?
                                 boolTrue : boolFalse));

            emit readStatusFinished(msg);
        }
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadAcknowledge
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadAcknowledge(const unsigned char *buffer, int n)
{
    if (n >= 8 ) {
        commandBuffer[static_cast<int>(Command::ReadAcknowledge)] = false;
        emit readAcknowledgeFinished(buffer[5], buffer[6]);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetLowTemperatureLimit
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetLowTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setLowTemperetareLimitFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadLowTemperatureLimit
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadLowTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iT = buffer[6] * 256 + buffer[7];
        if (iT >= negativeNumbersLimit)
            iT = iT - maxNumber;
        double dblT;

        if (buffer[5] & (1 << 5)) {
            enhancedBathPrecision_ = true;
            dblT = iT / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedBathPrecision_ = false;
            dblT = iT / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadLowTemperatureLimit)] = false;
        emit readLowTemperatureLimitFinished(dblT, enhancedBathPrecision_);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetHighTemperatureLimit
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetHighTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setHighTemperatureLimitFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadHighTemperatureLimit
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadHighTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iT = buffer[6] * 256 + buffer[7];
        if (iT >= negativeNumbersLimit)
            iT = iT - maxNumber;
        double dblT;

        if (buffer[5] & (1 << 5)) {
            enhancedBathPrecision_ = true;
            dblT = iT / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedBathPrecision_ = false;
            dblT = iT / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadHighTemperatureLimit)] = false;
        emit readHighTemperatureLimitFinished(dblT, enhancedBathPrecision_);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetHeatProportionalBand
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetHeatProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setHeatProportionalBandFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadHeatProportionalBand
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadHeatProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iP = buffer[6] * 256 + buffer[7];
        double dblP;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblP = iP / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblP = iP / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadHeatProportionalBand)] = false;
        emit readHeatProportionalBandFinished(dblP, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetHeatIntegral
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetHeatIntegral(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setHeatIntegralFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadHeatIntegral
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadHeatIntegral(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iI = buffer[6] * 256 + buffer[7];
        double dblI;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblI = iI / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblI = iI / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadHeatIntegral)] = false;
        emit readHeatIntegralFinished(dblI, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetHeatDerivative
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetHeatDerivative(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setHeatDerivativeFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadHeatDerivative
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadHeatDerivative(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iD = buffer[6] * 256 + buffer[7];
        double dblD;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblD = iD / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblD = iD / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadHeatDerivative)] = false;
        emit readHeatDerivativeFinished(dblD, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetCoolProportionalBand
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetCoolProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setCoolProportionalBandFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadCoolProportionalBand
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadCoolProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iP = buffer[6] * 256 + buffer[7];
        double dblP;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblP = iP / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblP = iP / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadCoolProportionalBand)] = false;
        emit readCoolProportionalBandFinished(dblP, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetCoolIntegral
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetCoolIntegral(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setCoolIntegralFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadCoolIntegral
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadCoolIntegral(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iI = buffer[6] * 256 + buffer[7];
        double dblI;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblI = iI / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblI = iI / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadCoolIntegral)] = false;
        emit readCoolIntegralFinished(dblI, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetCoolDerivative
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetCoolDerivative(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setCoolDerivativeFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadCoolDerivative
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadCoolDerivative(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iD = buffer[6] * 256 + buffer[7];
        double dblD;
        bool enhancedPrec;

        if (buffer[5] & (1 << 5)) {
            enhancedPrec = true;
            dblD = iD / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedPrec = false;
            dblD = iD / 10.0;
        } else {
            // bad precision byte
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadCoolDerivative)] = false;
        emit readCoolDerivativeFinished(dblD, enhancedPrec);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onReadSetpoint
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadSetpoint(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iT = buffer[6] * 256 + buffer[7];
        if (iT >= negativeNumbersLimit)
            iT = iT - maxNumber;
        double dblT;

        if (buffer[5] & (1 << 5)) {
            enhancedBathPrecision_ = true;
            dblT = iT / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedBathPrecision_ = false;
            dblT = iT / 10.0;
        } else {
            // bad precision bytes
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadSetpoint)] = false;
        emit readSetpointFinished(dblT, enhancedBathPrecision_);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onSetSetpoint
   \param buffer
   \param n
   \return
 */
bool Neslab::onSetSetpoint(const unsigned char *buffer, int n)
{
    if (n >= 9) {
        if (static_cast<unsigned char>(3) != buffer[4])
            return false;

        emit setSetpointFinished();
        return true;
    } else
        return false;
}

/*!
   \brief Neslab::onReadExternalSensor
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadExternalSensor(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iT = buffer[6] * 256 + buffer[7];
        if (iT >= negativeNumbersLimit)
            iT = iT - maxNumber;
        double dblT;

        if (buffer[5] & (1 << 5)) {
            enhancedBathPrecision_ = true;
            dblT = iT / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedBathPrecision_ = false;
            dblT = iT / 10.0;
        } else {
            // bad precision bytes
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadExternalSensor)] = false;
        emit readExternalSensorFinished(dblT, enhancedBathPrecision_);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::onReadInternalTemperature
   \param buffer
   \param n
   \return
 */
bool Neslab::onReadInternalTemperature(const unsigned char *buffer, int n)
{
    if (n >= 9 ) {
        int iT = buffer[6] * 256 + buffer[7];
        if (iT >= negativeNumbersLimit)
            iT = iT - maxNumber;
        double dblT;

        if (buffer[5] & (1 << 5)) {
            enhancedBathPrecision_ = true;
            dblT = iT / 100.0;
        } else if (buffer[5] & (1 << 4)) {
            enhancedBathPrecision_ = false;
            dblT = iT / 10.0;
        } else {
            // bad precision bytes
            return false;
        }
        commandBuffer[static_cast<int>(Command::ReadInternalTemperature)] = false;
        emit readInternalTemperatureFinished(dblT, enhancedBathPrecision_);
        return true;
    } else {
        // bad number of bytes
        return false;
    }
}

/*!
   \brief Neslab::hexToByte
   \param pbuffer
   \param n
   \param msg
 */
void Neslab::hexToByte(unsigned char **pbuffer, int *n, const QString &msg)
{
    // remove white spaces
    QString newMsg = msg;
    newMsg.remove(' ');

    int msgSize = newMsg.size();

    // test correct size of msg, all hex codes consit of 2 characters
    if (msgSize % 2) {
        *pbuffer = nullptr;
        *n = 0;
    } else {
        *n = msgSize / 2;
        *pbuffer = new unsigned char[*n];
        bool ok;
        for (int ii = 0; ii < *n; ++ii) {
            (*pbuffer)[ii] = newMsg.midRef(2 * ii, 2).toUInt(&ok, 16);
        }
    }
}

/*!
   \brief Neslab::byteToHex
   \param buffer
   \param n
   \return
 */
QString Neslab::byteToHex(const unsigned char *buffer, int n)
{
    QString msg;
    for (int ii = 0; ii < n - 1; ++ii) {
        msg.append(QString("%1").arg((unsigned int)buffer[ii], 2, 16, QChar('0')).toUpper()).append(' ');
    }
    if (n > 0) {
        msg.append(QString("%1").arg((unsigned int)buffer[n - 1], 2, 16, QChar('0')).toUpper());
    }
    return msg;
}

/*!
   \brief Neslab::checkSum
   \param msg
   \return
 */
QString Neslab::checkSum(const QString &msg)
{
    unsigned char *bMsg;
    int n;
    hexToByte(&bMsg, &n, msg);

    unsigned char bChk = checkSum(bMsg, n);

    delete[] bMsg;

    return QString("%1").arg((unsigned int)bChk, 2, 16, QChar('0')).toUpper();
}

/*!
   \brief Neslab::checkSum
   \param bMsg
   \param n
   \return
 */
unsigned char Neslab::checkSum(const unsigned char *bMsg, int n)
{
    unsigned int iSum = 0u;
    for (int ii = 1; ii < n; ++ii) {
        iSum += (unsigned int)bMsg[ii];
    }
    unsigned char byteSum = iSum % 256;
    return (unsigned char)(~byteSum);
}

/*!
   \brief Neslab::validateResponse
   \param msg
   \param cmd
   \return
 */
bool Neslab::validateResponse(const QString &msg, Command cmd)
{
    unsigned char *bMsg;
    int n;
    hexToByte(&bMsg, &n, msg);
    if (validateResponse(bMsg, n, cmd)) {
        delete[] bMsg;
        return true;
    } else {
        delete[] bMsg;
        return false;
    }
}

/*!
    \brief Neslab::validateResponse
    \param bMsg Pointer to field of bytes containing the response.
    \param n    The number of command bytes.
    \param cmd  The last command enum value.
    \return     true if response is valid, false if not.
*/
bool Neslab::validateResponse(const unsigned char *bMsg, int n, Command cmd)
{
    if (n >= 5) {
        for (int ii = 0; ii < 4; ++ii)
            if (bMsg[ii] != bCommands[static_cast<int>(cmd)][ii])
                return false;
        int m = bMsg[4];
        if (n >= 5 + m + 1) {
            unsigned char bTest[4 + m];
            for (int ii = 1; ii < 4 + m + 1; ++ii)
                bTest[ii - 1] = bMsg[ii];
            unsigned char bChkSum = checkSum(bTest, 4 + m);
            if ((unsigned int)bChkSum == (unsigned int)bMsg[5 + m + 1 - 1])
                return true;
        }
    }
    return false;
}

/*!
   \brief Fills arrays containing commands according to their importance.
 */
void Neslab::fillCommandsArrays()
{
    strCommands[static_cast<int>(Command::SetOnOff)] = wOnOffCmd;
    strCommands[static_cast<int>(Command::ReadStatus)] = rStatusCmd;
    strCommands[static_cast<int>(Command::ReadAcknowledge)] = rAcknowledgeCmd;
    strCommands[static_cast<int>(Command::SetLowTemperatureLimit)] = wLowTLimCmd;
    strCommands[static_cast<int>(Command::ReadLowTemperatureLimit)] = rLowTLimCmd;
    strCommands[static_cast<int>(Command::SetHighTemperatureLimit)] = wHigTLimCmd;
    strCommands[static_cast<int>(Command::ReadHighTemperatureLimit)] = rHighTLimCmd;
    strCommands[static_cast<int>(Command::SetHeatProportionalBand)] = wHeatProportionalBandCmd;
    strCommands[static_cast<int>(Command::ReadHeatProportionalBand)] = rHeatProportionalBandCmd;
    strCommands[static_cast<int>(Command::SetHeatIntegral)] = wHeatIntegraCmd;
    strCommands[static_cast<int>(Command::ReadHeatIntegral)] = rHeatIntegraCmd;
    strCommands[static_cast<int>(Command::SetHeatDerivative)] = wHeatDerivativeCmd;
    strCommands[static_cast<int>(Command::ReadHeatDerivative)] = rHeatDerivativeCmd;
    strCommands[static_cast<int>(Command::SetCoolProportionalBand)] = wCoolProportionalBandCmd;
    strCommands[static_cast<int>(Command::ReadCoolProportionalBand)] = rCoolProportionalBandCmd;
    strCommands[static_cast<int>(Command::SetCoolIntegral)] = wCoolIntegraCmd;
    strCommands[static_cast<int>(Command::ReadCoolIntegral)] = rCoolIntegraCmd;
    strCommands[static_cast<int>(Command::SetCoolDerivative)] = wCoolDerivativeCmd;
    strCommands[static_cast<int>(Command::ReadCoolDerivative)] = rCoolDerivativeCmd;
    strCommands[static_cast<int>(Command::ReadSetpoint)] = rSetPointCmd;
    strCommands[static_cast<int>(Command::SetSetpoint)] = wSetPointCmd;
    strCommands[static_cast<int>(Command::ReadExternalSensor)] = rExtSensorCmd;
    strCommands[static_cast<int>(Command::ReadInternalTemperature)] = rInternalTCmd;

    unsigned char *bBasMsg;
    int n;
    bool ok;
    hexToByte(&bBasMsg, &n, commandBase);
    for (int ii = 0; ii < static_cast<int>(Command::None); ++ii) {
        for (int jj = 0; jj < 3; ++jj) {
            bCommands[ii][jj] = bBasMsg[jj];
        }
        bCommands[ii][3] = static_cast<unsigned char>(strCommands[ii].toUInt(&ok, 16));
    }
    delete[] bBasMsg;
}

/*!
   \brief Neslab::~Neslab
 */
Neslab::~Neslab()
{
    serialPort->close();
    delete serialPort;
    delete cmdTimer;
    delete waitingTimer;
    delete mutex;
}

// documentation for private variables
/*!
    \var Neslab::comPortName
        \brief Name of used serial port.
    \var Neslab::commandBuffer
        \brief Array which stores commands waiting for execution.

        The commands are ordered according to their priority, first is the most
        important, last is the least. The meaning of particular array elements
        is determined by the NeslabTraits::Command enum.

        Example of determination whether the Set on/off command is scheduled:
        \code
        bool bSecheduled = commandBuffer[static_cast<int>(NeslabTraits::Command::SetOnOff)]
        \endcode

    \var Neslab::lastCommand
        \brief Stores information which command is currently in the process.

    \var Neslab::wSetpointT_
        \brief Stores desired setpoint when setSetpointCommand() is scheduled.
    \var Neslab::wLowTLim_
        \brief Stores desired low temperature limit when
        setLowTemperatureLimitCommand() is scheduled.
    \var Neslab::wHighTLim_
        \brief Stores desired high temperature limit when
        setHighTemperatureLimitCommand() is scheduled.
    \var Neslab::wHeatP_
        \brief Stores desired heat proportional band when
        setHeatProportionalBandCommand() is scheduled.
    \var Neslab::wHeatI_
        \brief Stores desired heat integral when setHeatIntegralCommand() is
        scheduled.
    \var Neslab::wHeatD_
        \brief Stores desired heat derivative when setHeatDerivativeCommand()
        is scheduled.
    \var Neslab::wCoolP_
        \brief Stores desired heat proportional band when
        setCoolProportionalBandCommand() is scheduled.
    \var Neslab::wCoolI_
        \brief Stores desired heat integral when setCoolIntegralCommand() is
        scheduled.
    \var Neslab::wCoolD_
        \brief Stores desired heat derivative when setCoolDerivativeCommand()
        is scheduled.
    \var Neslab::wOnOffParm_
        \brief Stores desired NeslabTraits::PowerOnOffParm when
        setOnOffCommand() is scheduled.
    \var Neslab::rOnOffParm_
        \brief Stores NeslabTraits::PowerOnOffParm which were lastly received
        from the bath.
    \var Neslab::bathStatus_
        \brief Stores NeslabTraits::BathStatus which was lastly received from
        the bath.
    \var Neslab::enhancedBathPrecision_
        \brief Stores if the enhanced bath precision (0.01) is used (true) or
        not (0.1).
    \var Neslab::connected_
        \brief Stores the bath connection status. Only connectNeslab() command
        can be send to the bath
        if the bath is not connected.
    \var Neslab::connecting_
        \brief Connect command is implemented as set on/off command and this
        variable is used to distinguish between the normal set on/off command
        and that used for checking connection.
    \var Neslab::switchingOn_
        \brief The unit is switched on by the set on/off command but this
        command is important enough to have its own method. This variable is
        used to distinguish between the normal set on/off command and that used
        for switching on.
    \var Neslab::switchingOff_
        \brief The unit is switched off by the set on/off command but this
        command is important enough to have its own method. This variable is
        used to distinguish between the normal set on/off command and that used
        for switching off.
    \var Neslab::readStatusSilently_
        \brief This variable is used to distinguis between readStatusCommand()
        (false) and Neslab::updateStatusCommand() (true). The first one emits
        Neslab::readStatusFinished() signal with the QString message with the
        bath status after the response from the bath is received whereas the
        second one emits statusUpdated() signal without any message.
    \var Neslab::statusUpdatedAfterOnOffCmd_
        \brief It determines if the set on/off command (sendSetOnOffCommand())
        is being currently executed and status has not been updated yet.
        \sa statusUpdatedAfterOnOffCmd()
    \var Neslab::waitingTimer
        \brief Pointer to the QTimer, which is used to stop delay after
        response for bath was received. This delay is controlled by the
        Neslab::cmdDelay static constant and it triggers the
        onWaitingFinished() function.
        \sa onReadyData()
    \var Neslab::waitingForNextCommand_
        \brief This varibale is true when the Neslab::waitingTimer runs
        (producing the delay between commands). When the new request for
        command comes, it is checked wheather is there any command running or
        if it is waiting for delay between consecutive commands and if not,
        command is send directly otherwise command is scheduled to the
        Neslab::commandBuffer.
        \sa onReadyData(), onWaitingFinished()
    \var Neslab::trialCount;
        \brief This variable stores the number of spare attempts to send
        current command. If it reaches zero the Neslab::connectionFailed()
        signal is emited.
        \sa Neslab::initTrialCount,
        validateResponse(const unsigned char *, int, NeslabTraits::Command),
        sendCommand(), onReadyData(), onNoResponse()
    \var Neslab::cmdTimer
        \brief The pointer to the QTimer, which is used to stop waiting for
        bath response after command had been sent. The longest waiting time is
        controlled by the Neslab::waitForResponse static constant and the
        onNoResponse() function is executed after the time is out.
    \var Neslab::mutex
        \brief provides the thread safety. It is locked in each public slot or
        function.
    \var Neslab::serialPort;
        \brief The pointer to the serial port object used to controll %Neslab
        thermostated bath. If the VIRTUALSERIALPORT is defined the
        VirtualSerialPort class which simulates the connected bath is used.
*/

/*!
   \class Neslab::MutexManager
   \brief Class which ensures that the static mutex pointer will be deleted at
   the end. It enables to forward declare QMutex even though it is static.
 */

/*!
   \brief Construts the instance of the class
 */
Neslab::MutexManager::MutexManager() :
    mutex(new QMutex)
{
}

/*!
   \fn QMutex * Neslab::MutexManager::getMutex() const
   \brief gets the pointer to the mutex
 */

/*!
  \var Neslab::MutexManager::mutex
  \brief variable storing the mutex
 */

/*!
   \brief Deletes the mutex at the end.
 */
Neslab::MutexManager::~MutexManager()
{
    delete mutex;
}


#ifdef VIRTUALSERIALPORT

const int VirtualSerialPort::cmdWait = 10;
const double VirtualSerialPort::speed_ = 1e-3;
std::default_random_engine VirtualSerialPort::engine;
unsigned char VirtualSerialPort::bCommands[static_cast<int>(Command::None)][4];
QString VirtualSerialPort::strCommands[static_cast<int>(Command::None)];
const QString VirtualSerialPort::commandBase = Neslab::commandBase;

VirtualSerialPort::VirtualSerialPort(QObject *parent) :
    QObject(parent)
{
    fillCommandsArrays();
    cmd_ = nullptr;
    n_ = 0;

    cmdWaiting_ = false;

    bathStatus.d1 = BathStatusTraits::d1::None;
    bathStatus.d2 = BathStatusTraits::d2::None;
    bathStatus.d3 = BathStatusTraits::d3::None;
    bathStatus.d4 = BathStatusTraits::d4::None;
    bathStatus.d5 = BathStatusTraits::d5::None;

    powerOnOffParm_[PowerOnOff::d1_unitOnOff] = 1;
    powerOnOffParm_[PowerOnOff::d2_sensorEnabled] = 1;
    powerOnOffParm_[PowerOnOff::d3_faultsEnabled] = 0;
    powerOnOffParm_[PowerOnOff::d4_mute] = 0;
    powerOnOffParm_[PowerOnOff::d5_autoRestart] = 0;
    powerOnOffParm_[PowerOnOff::d6_01precDegCEnable] = 0;
    powerOnOffParm_[PowerOnOff::d7_fullRangeCoolEnable] = 1;
    powerOnOffParm_[PowerOnOff::d8_serialCommEnable] = 1;

    if (powerOnOffParm_[PowerOnOff::d1_unitOnOff])
        bathStatus.d4 |= BathStatusTraits::d4::UnitOn;
    if (powerOnOffParm_[PowerOnOff::d2_sensorEnabled])
        bathStatus.d5 |= BathStatusTraits::d5::RTD2Controlling;
    if (powerOnOffParm_[PowerOnOff::d4_mute])
        bathStatus.d4 |= BathStatusTraits::d4::AlarmMuted;

    currExtT_ = 200;
    currIntT_ = 200;
    setTCmdT_ = 200;
    destT_    = 200;
    lowTLim_  = -50;
    highTLim_ = 990;
    heatP_ = 6; // 0.1  - 99.9
    heatI_ = 60; // 0.00 -  9.99
    heatD_ = 0; // 0.0  -  5.0
    coolP_ = 6; // 0.1  - 99.9
    coolI_ = 60; // 0.00 -  9.99
    coolD_ = 0; // 0.0  -  5.0

    faultsEnabled_ = true;
    autorestart_ = false;
    enhancedBathPrecision_ = false;
    serialCommEnabled_ = true;

    if (enhancedBathPrecision_) {
        currExtT_ = currExtT_ * 10;
        currIntT_ = currIntT_ * 10;
        setTCmdT_ = setTCmdT_ * 10;
        destT_    = destT_ * 10;
        lowTLim_  = lowTLim_ * 10;
        highTLim_ = highTLim_ * 10;
    }

    setTCmdTime_ = new QDateTime;
    cmdTimer_ = new QTimer(this);
    connect(cmdTimer_, &QTimer::timeout, this, &VirtualSerialPort::onCmdWaitFinished);

    engine.seed(time(0));
}

void VirtualSerialPort::setPortName(const QString &comPortName)
{
    portName_ = comPortName;
}

void VirtualSerialPort::setBaudRate(qint32 baudRate, Directions directions)
{
    baudRate_ = baudRate;
    baudRateDirections_ = directions;
}

void VirtualSerialPort::setDataBits(DataBits dataBits)
{
    dataBits_ = dataBits;
}

void VirtualSerialPort::setParity(Parity parity)
{
    parity_ = parity;
}

void VirtualSerialPort::setStopBits(StopBits stopBits)
{
    stopBits_ = stopBits;
}

void VirtualSerialPort::setFlowControl(FlowControl flowControl)
{
    flowControl_ = flowControl;
}

void VirtualSerialPort::close()
{
    opened_ = false;
}

QByteArray VirtualSerialPort::readAll()
{
    QByteArray data(reinterpret_cast<char *>(cmd_), n_);
    delete[] cmd_;
    cmd_ = nullptr;
    n_ = 0;
    return data;
}

qint64 VirtualSerialPort::write(char *data, qint64 len)
{
    if (cmdWaiting_)
        return -1;
    cmdWaiting_ = true;
    delete[] cmd_;
    cmd_ = nullptr;
    n_ = 0;
    unsigned char * cmd = reinterpret_cast<unsigned char *>(data);

    bool ok = false;

    if (validateResponse(cmd, len))
    {
        if (cmd[3] == bCommands[static_cast<int>(Command::SetOnOff)][3])
            ok = onPowerOnOff(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadStatus)][3])
            ok = onReadStatus(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadAcknowledge)][3])
            ok = onReadAcknowledge(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetLowTemperatureLimit)][3])
            ok = onSetLowTemperatureLimit(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadLowTemperatureLimit)][3])
            ok = onReadLowTemperatureLimit(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetHighTemperatureLimit)][3])
            ok = onSetHighTemperatureLimit(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadHighTemperatureLimit)][3])
            ok = onReadHighTemperatureLimit(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetHeatProportionalBand)][3])
            ok = onSetHeatProportionalBand(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadHeatProportionalBand)][3])
            ok = onReadHeatProportionalBand(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetHeatIntegral)][3])
            ok = onSetHeatIntegral(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadHeatIntegral)][3])
            ok = onReadHeatIntegral(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetHeatDerivative)][3])
            ok = onSetHeatDerivative(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadHeatDerivative)][3])
            ok = onReadHeatDerivative(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetCoolProportionalBand)][3])
            ok = onSetCoolProportionalBand(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadCoolProportionalBand)][3])
            ok = onReadCoolProportionalBand(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetCoolIntegral)][3])
            ok = onSetCoolIntegral(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadCoolIntegral)][3])
            ok = onReadCoolIntegral(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetCoolDerivative)][3])
            ok = onSetCoolDerivative(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadCoolDerivative)][3])
            ok = onReadCoolDerivative(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadSetpoint)][3])
            ok = onReadSetpoint(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::SetSetpoint)][3])
            ok = onSetSetpoint(cmd, len);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadExternalSensor)][3])
            ok = onReadExternalSensor(cmd);
        else if (cmd[3] == bCommands[static_cast<int>(Command::ReadInternalTemperature)][3])
            ok = onReadInternalTemperature(cmd);
        if (!ok) {
            // command is not ok
            onBadCommand(cmd, len);
        }
    }
    else
    {
        // data is invalid
        onBadCheckSum(cmd, len);
    }
    cmdTimer_->setSingleShot(true);
    cmdTimer_->start(cmdWait);
    return len;
}

void VirtualSerialPort::onCmdWaitFinished()
{
    cmdWaiting_ = false;

    std::uniform_int_distribution<int> dist(0,100);

    int ri = dist(engine);
    if (!ri)
    {
        std::uniform_int_distribution<int> dist1(0,n_ - 1);
        std::uniform_int_distribution<int> dist2(0,255);
        int ii = dist1(engine);
        unsigned char val = dist2(engine);
        qDebug() << "VirtualSerialPort: sending faulted, replacement of byte" << ii << "by the value"
                 << QString("%1").arg((unsigned int)val, 2, 16, QChar('0')).toUpper();
        cmd_[ii] = val;
    }

    if (ri == 1)
        qDebug() << "The message disapeared.";
    else
        emit readyRead();
}

bool VirtualSerialPort::onPowerOnOff(const unsigned char *buffer, int n)
{
    n_ = n;
    cmd_ = new unsigned char[n_];
    if (n >= 14) {
        PowerOnOffParm powerOnOffParm = powerOnOffParm_;
        for (int ii = 0; ii < 8; ++ii)
            if (buffer[ii + 5] < 2)
                powerOnOffParm[ii] = buffer[ii + 5];

        if (powerOnOffParm[PowerOnOff::d1_unitOnOff] != powerOnOffParm_[PowerOnOff::d1_unitOnOff]) {
            if (powerOnOffParm[PowerOnOff::d1_unitOnOff]) {
                bathStatus.d4 |= BathStatusTraits::d4::UnitOn;
            } else {
                bathStatus.d4 &= ~BathStatusTraits::d4::UnitOn;
            }
        }
        if (powerOnOffParm[PowerOnOff::d2_sensorEnabled] != powerOnOffParm_[PowerOnOff::d2_sensorEnabled]) {
            if (powerOnOffParm[PowerOnOff::d2_sensorEnabled]) {
                currExtT_ = currIntT_;
                bathStatus.d5 |= BathStatusTraits::d5::RTD2Controlling;
            } else {
                currIntT_ = currExtT_;
                bathStatus.d5 &= ~BathStatusTraits::d5::RTD2Controlling;
            }
        }
        if (powerOnOffParm[PowerOnOff::d3_faultsEnabled] != powerOnOffParm_[PowerOnOff::d3_faultsEnabled]) {
            if (powerOnOffParm[PowerOnOff::d3_faultsEnabled]) {
            } else {
            }
        }
        if (powerOnOffParm[PowerOnOff::d4_mute] != powerOnOffParm_[PowerOnOff::d4_mute]) {
            if (powerOnOffParm[PowerOnOff::d4_mute]) {
                bathStatus.d4 |= BathStatusTraits::d4::AlarmMuted;
            } else {
                bathStatus.d4 &= ~BathStatusTraits::d4::AlarmMuted;
            }
        }
        if (powerOnOffParm[PowerOnOff::d5_autoRestart] != powerOnOffParm_[PowerOnOff::d5_autoRestart]) {
            if (powerOnOffParm[PowerOnOff::d5_autoRestart]) {
            } else {
            }
        }
        if (powerOnOffParm[PowerOnOff::d6_01precDegCEnable] != powerOnOffParm_[PowerOnOff::d6_01precDegCEnable]) {
            if (powerOnOffParm[PowerOnOff::d6_01precDegCEnable]) {
                destT_ = destT_ * 10;
                currExtT_ = currExtT_ * 10;
                currIntT_ = currIntT_ * 10;
                setTCmdT_ = setTCmdT_ * 10;
                lowTLim_ = lowTLim_ * 10;
                highTLim_ = highTLim_ * 10;
                enhancedBathPrecision_ = true;
            } else {
                destT_ = (destT_ + 5) / 10;
                currExtT_ = (currExtT_ + 5) / 10;
                currIntT_ = (currIntT_ + 5) / 10;
                setTCmdT_ = (setTCmdT_ + 5) / 10;
                lowTLim_ = (lowTLim_ + 5) / 10;
                highTLim_ = (highTLim_ + 5) / 10;
                enhancedBathPrecision_ = false;
            }
        }
        if (powerOnOffParm[PowerOnOff::d7_fullRangeCoolEnable] != powerOnOffParm_[PowerOnOff::d7_fullRangeCoolEnable]) {
            if (powerOnOffParm[PowerOnOff::d7_fullRangeCoolEnable]) {
                bathStatus.d2 |= BathStatusTraits::d2::RefrigHighTemp;
            } else {
                bathStatus.d2 &= ~BathStatusTraits::d2::RefrigHighTemp;
            }
        }
        if (powerOnOffParm[PowerOnOff::d8_serialCommEnable] != powerOnOffParm_[PowerOnOff::d8_serialCommEnable]) {
            if (powerOnOffParm[PowerOnOff::d8_serialCommEnable]) {
            } else {
            }
        }
        powerOnOffParm_ = powerOnOffParm;
        for (int ii = 0; ii < 5; ++ii)
            cmd_[ii] = static_cast<unsigned char>(buffer[ii]);
        for (int ii = 0; ii < 8; ++ii)
            cmd_[ii + 5] = powerOnOffParm_[ii];
        cmd_[13] = Neslab::checkSum(cmd_, 13);
    } else {
        for (int ii = 0; ii < n_; ++ii)
            cmd_[ii] = static_cast<unsigned char>(buffer[ii]);
    }
    return true;
}

bool VirtualSerialPort::onReadStatus(const unsigned char *buffer)
{
    n_ = 11;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 5;
    cmd_[5] = static_cast<unsigned char>(bathStatus.d1);
    cmd_[6] = static_cast<unsigned char>(bathStatus.d2);
    cmd_[7] = static_cast<unsigned char>(bathStatus.d3);
    cmd_[8] = static_cast<unsigned char>(bathStatus.d4);
    cmd_[9] = static_cast<unsigned char>(bathStatus.d5);
    cmd_[10] = Neslab::checkSum(cmd_, 10);
    return true;
}

bool VirtualSerialPort::onReadAcknowledge(const unsigned char *buffer)
{
    n_ = 8;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 2;
    cmd_[5] = 3;
    cmd_[6] = 14;
    cmd_[7] = Neslab::checkSum(cmd_, 7);
    return true;
}

bool VirtualSerialPort::onSetLowTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        int iTInc = buffer[5] * 256 + buffer[6];
        if (iTInc >= Neslab::negativeNumbersLimit)
            iTInc = iTInc - Neslab::maxNumber;
        lowTLim_ = iTInc;

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        if (enhancedBathPrecision_)
            cmd_[5] = (1 << 5) + 1;
        else
            cmd_[5] = (1 << 4) + 1;

        int iT = lowTLim_;
        if (iT < 0) {
            iT = Neslab::maxNumber + iT;
        }

        cmd_[6] = lowTLim_ / 256;
        cmd_[7] = lowTLim_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadLowTemperatureLimit(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    if (enhancedBathPrecision_)
        cmd_[5] = (1 << 5) + 1;
    else
        cmd_[5] = (1 << 4) + 1;

    int iT = lowTLim_;
    if (iT < 0) {
        iT = Neslab::maxNumber + iT;
    }

    cmd_[6] = iT / 256;
    cmd_[7] = iT % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetHighTemperatureLimit(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        int iTInc = buffer[5] * 256 + buffer[6];
        if (iTInc >= Neslab::negativeNumbersLimit)
            iTInc = iTInc - Neslab::maxNumber;
        highTLim_ = iTInc;

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        if (enhancedBathPrecision_)
            cmd_[5] = (1 << 5) + 1;
        else
            cmd_[5] = (1 << 4) + 1;

        int iT = highTLim_;
        if (iT < 0) {
            iT = Neslab::maxNumber + iT;
        }

        cmd_[6] = iT / 256;
        cmd_[7] = iT % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadHighTemperatureLimit(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    if (enhancedBathPrecision_)
        cmd_[5] = (1 << 5) + 1;
    else
        cmd_[5] = (1 << 4) + 1;

    int iT = highTLim_;
    if (iT < 0) {
        iT = Neslab::maxNumber + iT;
    }

    cmd_[6] = iT / 256;
    cmd_[7] = iT % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetHeatProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        heatP_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 4);
        cmd_[6] = heatP_ / 256;
        cmd_[7] = heatP_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadHeatProportionalBand(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 4);
    cmd_[6] = heatP_ / 256;
    cmd_[7] = heatP_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetHeatIntegral(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        heatI_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 5);
        cmd_[6] = heatI_ / 256;
        cmd_[7] = heatI_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadHeatIntegral(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 5);
    cmd_[6] = heatI_ / 256;
    cmd_[7] = heatI_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetHeatDerivative(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        heatD_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 5);
        cmd_[6] = heatD_ / 256;
        cmd_[7] = heatD_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadHeatDerivative(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 5);
    cmd_[6] = heatD_ / 256;
    cmd_[7] = heatD_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetCoolProportionalBand(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        coolP_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 4);
        cmd_[6] = coolP_ / 256;
        cmd_[7] = coolP_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadCoolProportionalBand(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 4);
    cmd_[6] = coolP_ / 256;
    cmd_[7] = coolP_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetCoolIntegral(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        coolI_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 5);
        cmd_[6] = coolI_ / 256;
        cmd_[7] = coolI_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadCoolIntegral(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 5);
    cmd_[6] = coolI_ / 256;
    cmd_[7] = coolI_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetCoolDerivative(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        coolD_ = buffer[5] * 256 + buffer[6];

        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        cmd_[5] = (1 << 5);
        cmd_[6] = coolD_ / 256;
        cmd_[7] = coolD_ % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadCoolDerivative(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    cmd_[5] = (1 << 5);
    cmd_[6] = coolD_ / 256;
    cmd_[7] = coolD_ % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onReadSetpoint(const unsigned char *buffer)
{
    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    if (enhancedBathPrecision_)
        cmd_[5] = (1 << 5) + 1;
    else
        cmd_[5] = (1 << 4) + 1;

    int iT = destT_;
    if (iT < 0) {
        iT = Neslab::maxNumber + iT;
    }

    cmd_[6] = iT / 256;
    cmd_[7] = iT % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onSetSetpoint(const unsigned char *buffer, int n)
{
    if (n >= 8) {
        int iTinc = buffer[5] * 256 + buffer[6];
        if (iTinc >= Neslab::negativeNumbersLimit)
            iTinc = iTinc - Neslab::maxNumber;
        destT_ = iTinc;

        *setTCmdTime_ = QDateTime::currentDateTime();
        if (static_cast<bool>(bathStatus.d5 & BathStatusTraits::d5::RTD2Controlling))
            setTCmdT_ = currExtT_;
        else
            setTCmdT_ = currIntT_;
        if (destT_ > setTCmdT_)
            direction_ = 1;
        else
            direction_ = -1;
        n_ = 9;
        cmd_ = new unsigned char[n_];
        for (int ii = 0; ii < 4; ++ii)
            cmd_[ii] = buffer[ii];
        cmd_[4] = 3;
        if (enhancedBathPrecision_)
            cmd_[5] = (1 << 5) + 1;
        else
            cmd_[5] = (1 << 4) + 1;

        int iT = destT_;
        if (iT < 0) {
            iT = Neslab::maxNumber + iT;
        }

        cmd_[6] = iT / 256;
        cmd_[7] = iT % 256;
        cmd_[8] = Neslab::checkSum(cmd_, 8);
        return true;
    } else
        return false;
}

bool VirtualSerialPort::onReadExternalSensor(const unsigned char *buffer)
{
    if ((powerOnOffParm_[PowerOnOff::d2_sensorEnabled] == 1) && currExtT_ != destT_)
    {
        double speed;
        if (enhancedBathPrecision_)
            speed = speed_ * 100.0;
        else
            speed = speed_ * 10.0;
        currExtT_ = setTCmdT_ + direction_ *
                (QDateTime::currentDateTime().toMSecsSinceEpoch() - setTCmdTime_->toMSecsSinceEpoch()) * speed + 0.5;

        if ((currExtT_ > destT_ && setTCmdT_ < destT_) || (currExtT_ < destT_ && setTCmdT_ > destT_))
            currExtT_ = destT_;
    }

    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    if (enhancedBathPrecision_)
        cmd_[5] = (1 << 5) + 1;
    else
        cmd_[5] = (1 << 4) + 1;

    int iT = currExtT_;
    if (iT < 0) {
        iT = Neslab::maxNumber + iT;
    }

    cmd_[6] = iT / 256;
    cmd_[7] = iT % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

bool VirtualSerialPort::onReadInternalTemperature(const unsigned char *buffer)
{
    if ((powerOnOffParm_[PowerOnOff::d2_sensorEnabled] != 1) && currIntT_ != destT_)
    {
        double speed;
        if (enhancedBathPrecision_)
            speed = speed_ * 100.0;
        else
            speed = speed_ * 10.0;
        currIntT_ = setTCmdT_ + direction_ *
                (QDateTime::currentDateTime().toMSecsSinceEpoch() - setTCmdTime_->toMSecsSinceEpoch()) * speed + 0.5;

        if ((currIntT_ > destT_ && setTCmdT_ < destT_) || (currIntT_ < destT_ && setTCmdT_ > destT_))
            currIntT_ = destT_;
    }

    n_ = 9;
    cmd_ = new unsigned char[n_];
    for (int ii = 0; ii < 4; ++ii)
        cmd_[ii] = buffer[ii];
    cmd_[4] = 3;
    if (enhancedBathPrecision_)
        cmd_[5] = (1 << 5) + 1;
    else
        cmd_[5] = (1 << 4) + 1;

    int iT = currIntT_;
    if (iT < 0) {
        iT = Neslab::maxNumber + iT;
    }

    cmd_[6] = iT / 256;
    cmd_[7] = iT % 256;
    cmd_[8] = Neslab::checkSum(cmd_, 8);
    return true;
}

void VirtualSerialPort::onBadCheckSum(const unsigned char *buffer, int n)
{
    n_ = 8;
    cmd_ = new unsigned char[n_];

    unsigned char *bMsg;
    int m;
    if (n >= 6) {
        // bad checksum
        Neslab::hexToByte(&bMsg, &m, QString(Neslab::commandBase).append(Neslab::badCheckSumCmd));
        cmd_[6] = buffer[3];
    } else {
        // bad command
        Neslab::hexToByte(&bMsg, &m, QString(Neslab::commandBase).append(Neslab::badCommandCmd).append(" ").append(Neslab::badCommandCmd.midRef(0,2)));
    }

    for (int ii = 0; ii < m; ++ii)
        cmd_[ii] = bMsg[ii];

    delete[] bMsg;
    cmd_[7] = Neslab::checkSum(cmd_, 7);
}

void VirtualSerialPort::onBadCommand(const unsigned char *buffer, int n)
{
    n_ = 8;
    cmd_ = new unsigned char[n_];

    unsigned char *bMsg;
    int m;
    if (n >= 4) {
        Neslab::hexToByte(&bMsg, &m, QString(Neslab::commandBase).append(Neslab::badCommandCmd));
        cmd_[6] = buffer[3];
    } else {
        Neslab::hexToByte(&bMsg, &m, QString(Neslab::commandBase).append(Neslab::badCommandCmd).append(" ").append(Neslab::badCommandCmd.midRef(0,2)));
    }

    for (int ii = 0; ii < m; ++ii)
        cmd_[ii] = bMsg[ii];

    delete[] bMsg;
    cmd_[7] = Neslab::checkSum(cmd_, 7);
}

bool VirtualSerialPort::validateResponse(const QString &msg)
{
    unsigned char *bMsg;
    int n;
    Neslab::hexToByte(&bMsg, &n, msg);
    if (validateResponse(bMsg, n)) {
        delete[] bMsg;
        return true;
    } else {
        delete[] bMsg;
        return false;
    }
}

bool VirtualSerialPort::validateResponse(const unsigned char *bMsg, int n)
{
    std::uniform_int_distribution<int> dist(0,100);

    int ri = dist(engine);
    if (!ri)
    {
        qDebug() << "Reject";
        return false;
    }
    if (n >= 5) {
        int m = bMsg[4];
        if (n >= 5 + m + 1) {
            unsigned char bTest[4 + m];
            for (int ii = 1; ii < 4 + m + 1; ++ii)
                bTest[ii - 1] = bMsg[ii];
            unsigned char bChkSum = Neslab::checkSum(bTest, 4 + m);
            if ((unsigned int)bChkSum == (unsigned int)bMsg[5 + m + 1 - 1])
                return true;
        }
    }
    return false;
}

void VirtualSerialPort::fillCommandsArrays()
{
    for (int ii = 0; ii < static_cast<int>(Command::None); ++ii) {
        strCommands[ii] = Neslab::strCommands[ii];
        for (int jj = 1; jj < 4; ++jj)
            bCommands[ii][jj] = Neslab::bCommands[ii][jj];
    }
}

VirtualSerialPort::~VirtualSerialPort()
{
    delete setTCmdTime_;
    delete cmdTimer_;
    delete[] cmd_;
}

#endif // VIRTUALSERIALPORT
