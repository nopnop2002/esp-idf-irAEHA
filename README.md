# esp-idf-irAEHA
M5Stick and M5StickC as a remote control transmitter.   
This is AEHA IR Format exclusive use.   
AEHA IR Format is used only in Japan.   

ESP-IDF�ɂ�[������](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt_nec_tx_rx)�ɐԊO������M�̃T���v�����t�����Ă��܂����ANEC�t�H�[�}�b�g�����Ή����Ă��܂���B   

�䂪�Ƃ̃e���r�͉Ɛ���(AEHA)�t�H�[�}�b�g�Ȃ̂ŁA�Ɛ���(AEHA)�t�H�[�}�b�g�ɑΉ������A�v���P�[�V���������܂����B   
M5Stick M5StickC�������R���̑��M�@�Ƃ��Ďg�����Ƃ��ł��܂��B   

---

# �����R���R�[�h���
�����R���R�[�h�̉�͂ɂ�[rmt_aeha_rx](https://github.com/nopnop2002/esp-idf-irAEHA/tree/master/rmt_aeha_rx)���g���܂��B   

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/rmt_aeha_rx
idf.py menuconfig
idf.py flash monitor
```

�ԊO����M���W���[����GPIO19�ɐڑ����܂��B
�ԊO�����M�@�͎g�p���܂���B
GPIO19��ύX�������ꍇ�́A�ȉ���ύX���邱�ƂŔC�ӂ�GPIO���g�p���邱�Ƃ��ł��܂��B

```
#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
```


�t�@�[�����r���h���ă{�[�h�ɏ������񂾂�A���j�^�[�c�[�������s���ăV���A���o�͂�\�����܂��B
�����R���̃{�^���������ƁA��M�����ԊO���R�[�h���\������܂��B
������̓e���r�̓d���{�^�����������Ƃ��̕\���ł��B   

```
I (23850) AEHA: RMT RCV --- customer: 0x2002 parity: 0x00 index: 4
I (23850) AEHA: RMT RCV --- data[0]: 0x08
I (23860) AEHA: RMT RCV --- data[1]: 0x00
I (23860) AEHA: RMT RCV --- data[2]: 0x3d
I (23870) AEHA: RMT RCV --- data[3]: 0xbd
```

# �ԊO���R�[�h�̓o�^
���̂悤�ɂ��ĉ�͂����ԊO���R�[�h��font�f�B���N�g����Display.def�ɓo�^���܂��B   
Display.def�̏����͈ȉ��̒ʂ�ł��B   

```
�\������e�L�X�g,customer�R�[�h,parity�R�[�h,data�R�[�h�̐�,data0,data1,data2......;
```

*AEHA�t�H�[�}�b�g�̃p���e�B�R�[�h�́A�{���A�J�X�^�}�[�R�[�h��4�r�b�g�P�ʂ�XOR���Ƃ������̂ł����A
���ׂĂ݂�ƃp���e�B�ł͂Ȃ��A���ʂ�1�Ԗڂ̃f�[�^�Ƃ��Ďg���Ă��郊���R��������悤�ł��B   
�����ŁA���M���̃p���e�B�R�[�h�̓��C�u���������Ŏ����v�Z�����ɁA�O������^����悤�ɂ��Ă��܂��B



```
#This is define file for isp-idf-irAEHA
#Text,customer,parity,number_of_data,data0,data1,data2.....;
Power,0x2002,0x00,4,0x08,0x00,0x3d,0xbd;	TV on/off
Ch 1,0x2002,0x00,4,0x08,0x09,0x40,0xc9;		Channel 1
Ch 2,0x2002,0x00,4,0x08,0x09,0x41,0xc8;		Channel 2
Ch 3,0x2002,0x00,4,0x08,0x09,0x42,0xcb;		Channel 3
Ch 4,0x2002,0x00,4,0x08,0x09,0x43,0xca;		Channel 4
Ch 5,0x2002,0x00,4,0x08,0x09,0x44,0xcd;		Channel 5
Ch 6,0x2002,0x00,4,0x08,0x09,0x45,0xcc;		Channel 6
```


