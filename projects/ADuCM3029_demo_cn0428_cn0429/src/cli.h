/**
******************************************************************************
*   @file     cli.h
*   @brief    Command line interface header file
*   @version  V0.1
*   @author   ADI
*
*******************************************************************************
*
*Copyright 2015-2018(c) Analog Devices, Inc.
*
*All rights reserved.
*
*Redistribution and use in source and binary forms, with or without modification,
*are permitted provided that the following conditions are met:
*    - Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*    - Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in
*      the documentation and/or other materials provided with the
*      distribution.
*    - Neither the name of Analog Devices, Inc. nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*    - The use of this software may or may not infringe the patent rights
*      of one or more patent holders.  This license does not release you
*      from the requirement that you obtain separate licenses from these
*      patent holders to use this software.
*    - Use of the software either in source or binary form, must be run
*      on or directly connected to an Analog Devices Inc. component.
*
*THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
*PARTICULAR PURPOSE ARE DISCLAIMED.
*
*IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
*RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
*BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
**/

#ifndef CLI_H_
#define CLI_H_


/******************************  Variables  **********************************/
extern uint8_t		cmdReceived;

/****************************** Internal types ********************************/

typedef  void (*cmdFunc)(uint8_t *);

/*************************** Functions prototypes *****************************/
void DoNothing();
void CmdHelp(uint8_t *args);
uint8_t *FindArgv(uint8_t *args);
void GetArgv(char *dst, uint8_t *args);
void CmdProcess(void);
void CmdPrompt(void);
cmdFunc FindCommand(char *cmd);
void SetDefaultSns(uint8_t *args);
void SnsConnected(uint8_t *args);
void RdCfgs(uint8_t *args);
void RdTemp(uint8_t *args);
void RdHum(uint8_t *args);
void CfgMeasTime(uint8_t *args);
void StartMeas(uint8_t *args);
void StopMeas(uint8_t *args);
void CfgRtia(uint8_t *args);
void CfgRload(uint8_t *args);
void CfgVbias(uint8_t *args);
void CfgSens(uint8_t *args);
void CfgTempComp(uint8_t *args);
void RunEIS(uint8_t *args);
void RdEIS(uint8_t *args);
void RdEISfull(uint8_t *args);
void RdRcal(uint8_t *args);
void RunPulse(uint8_t *args);
void ReadPulse(uint8_t *args);
void SetPulseAmpl(uint8_t *args);
void SetPulseDuration(uint8_t *args);
void RdSensors(uint8_t *args);
void StopRd(uint8_t *args);
void CfgUpdateRate(uint8_t *args);






#endif /* CLI_H_ */
