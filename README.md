# esp-idf-irAEHA
M5Stick and M5StickC as a remote control transmitter.   
This is AEHA IR Format exclusive use.   
AEHA IR Format is used only in Japan.   

ESP-IDF�ɂ�[������](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/ir_nec_transceiver)�ɐԊO������M�̃T���v�����t�����Ă��܂����A
NEC�t�H�[�}�b�g�����Ή����Ă��܂���B   

�䂪�Ƃ̃e���r�͉Ɛ���(AEHA)�t�H�[�}�b�g�Ȃ̂ŁA�Ɛ���(AEHA)�t�H�[�}�b�g�ɑΉ������A�v���P�[�V���������܂����B   
M5Stick M5StickC(+)�������R���̑��M�@�Ƃ��Ďg�����Ƃ��ł��܂��B   


# Software requirements
ESP-IDF V4.4.   
ESP-IDF V5 �ł� RMT �h���C�o�[�̎d�l���啝�ɕύX����܂����B    


# �����R���R�[�h���
�����R���R�[�h�̉�͂ɂ� esp-idf-irAEHA-analysis ���g���܂��B   

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/esp-idf-irAEHA-analysis
idf.py menuconfig
idf.py flash monitor
```

�ԊO����M���W���[����GPIO19�ɐڑ����܂��B
�ԊO�����M�@�͎g�p���܂���B
GPIO19��ύX�������ꍇ�́Amenuconfig �ŔC�ӂ�GPIO�ɕύX���邱�Ƃ��ł��܂��B


�t�@�[�����r���h���ă{�[�h�ɏ������񂾂�A���j�^�[�c�[�������s���ăV���A���o�͂�\�����܂��B   
�����R���̃{�^���������ƁA��M�����ԊO���R�[�h���\������܂��B   
������̓e���r�̓d���{�^�����������Ƃ��̕\���ł��B   
�Ȃ����A�R�񓯂��R�[�h�����˂���Ă��邱�Ƃ�������܂��B   

```
I (6770) AEHA: Scan success
I (6770) AEHA: Scan Data[0]: 0x02
I (6770) AEHA: Scan Data[1]: 0x20
I (6770) AEHA: Scan Data[2]: 0x80
I (6780) AEHA: Scan Data[3]: 0x00
I (6780) AEHA: Scan Data[4]: 0x3d
I (6780) AEHA: Scan Data[5]: 0xbd
I (6900) AEHA: Scan success
I (6910) AEHA: Scan Data[0]: 0x02
I (6910) AEHA: Scan Data[1]: 0x20
I (6910) AEHA: Scan Data[2]: 0x80
I (6910) AEHA: Scan Data[3]: 0x00
I (6910) AEHA: Scan Data[4]: 0x3d
I (6920) AEHA: Scan Data[5]: 0xbd
I (7040) AEHA: Scan success
I (7040) AEHA: Scan Data[0]: 0x02
I (7040) AEHA: Scan Data[1]: 0x20
I (7040) AEHA: Scan Data[2]: 0x80
I (7040) AEHA: Scan Data[3]: 0x00
I (7050) AEHA: Scan Data[4]: 0x3d
I (7050) AEHA: Scan Data[5]: 0xbd
```

# �ԊO���R�[�h�̓o�^
���̂悤�ɂ��ĉ�͂����ԊO���R�[�h��font�f�B���N�g����Display.def�ɓo�^���܂��B   
Display.def�̏����͈ȉ��̒ʂ�ł��B   
�s�̏I�[�̓Z�~�R�����ł��B   

```
�\������e�L�X�g,���˂����,data0,data1,data2......;Comment
```

data0��data1��Customer Code�ƌĂ΂�Ă��܂��B   
data2��Parity Code�ƌĂ΂�Ă��܂��B   
Parity Code�́A�{���ACustomer Code��4�r�b�g�P�ʂ�XOR��������l�ł����A
���ׂĂ݂�Ƃ��̃��[���ɂ͏]�킸�A���ʂ�1�Ԗڂ̃f�[�^�Ƃ��Ďg���Ă��郊���R��������悤�ł��B   
�����ŁAParity Code�̓��C�u���������Ŏ����v�Z�����ɁA�O������^����悤�ɂ��Ă��܂��B



```
#This is define file for isp-idf-irAEHA
#Text,number of fire,data0,data1,data2.....;
Power ON/OFF,3,0x02,0x20,0x80,0x00,0x3d,0xbd; TV on/off
TV Channel 1,3,0x02,0x20,0x80,0x09,0x40,0xc9; Channel 1
TV Channel 2,3,0x02,0x20,0x80,0x09,0x41,0xc8; Channel 2
TV Channel 3,3,0x02,0x20,0x80,0x09,0x42,0xcb; Channel 3
TV Channel 4,3,0x02,0x20,0x80,0x09,0x43,0xca; Channel 4
TV Channel 5,3,0x02,0x20,0x80,0x09,0x44,0xcd; Channel 5
TV Channel 6,3,0x02,0x20,0x80,0x09,0x45,0xcc; Channel 6
TV Channel 7,3,0x02,0x20,0x80,0x09,0x46,0xcf; Channel 7
TV Channel 8,3,0x02,0x20,0x80,0x09,0x47,0xce; Channel 8
TV Channel 9,3,0x02,0x20,0x80,0x09,0x48,0xc1; Channel 9
TV Channel 10,3,0x02,0x20,0x80,0x09,0x49,0xc0; Channel 10
TV Channel 11,3,0x02,0x20,0x80,0x09,0x4a,0xc3; Channel 11
TV Channel 12,3,0x02,0x20,0x80,0x09,0x4b,0xc2; Channel 12
```


