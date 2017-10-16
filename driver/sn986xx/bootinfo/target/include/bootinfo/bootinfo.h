/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/**
 * @file
 *
 * Declaration of public functions that handle
 * the board's BOOTINFO controllers.
 *
 * @author Delezue
 */



int get_rtos_run_status(void);
void get_bootsel_info(void);
int timing_init(void);
int timing_uninit(void);
int timing_reinit(void);
int enter_lpwr_critical_section(int gpn);
unsigned int get_rtos_ver_sz_in_rescue(void);
char* get_rtos_ver_str_in_rescue(void);
int forced_to_rescue(void);
int clear_forced_to_rescue_flag(void);
