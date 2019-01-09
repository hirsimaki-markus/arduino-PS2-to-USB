#include <Keyboard.h>
#include <Mouse.h>

const byte dataPin = 2;
const byte irqPin = 3;

// ISR variable block
volatile uint8_t bitCounter = 0;         // counts 11 bit chunks to dataIn
volatile uint16_t dataIn = 0;            // accumulates bytes from keyboard
volatile static unsigned long reset = 0; // bitCounter & dataIn timeout if error
volatile bool errorFlag = false;         // raised if dataIn was erroneous
volatile uint8_t newBuffer = 0;          // stores latest byte sent by keyboard

// written into by parseNewBuffer() and read from by simulateKey()
uint8_t scanCodeType = 0; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
uint8_t scanCode = 0; // latest code to make or break. filled by parsenewbuffer and read by simulatekey

void DEBUG_HEXPRINT(uint8_t number) {
  /** 
   * prefixes numbers in range 0-255 with 0x and pads zero if needed to print
   * hex valus in preferred from
   */
  Serial.print("0x");
  if (number <= 15) Serial.print("0");
  Serial.println(number, HEX);
}

void simulateKey(void) {
  /**
   * this function simulates keypresses and keeps track of modifier keys, global variables used: scanCode scanCodeType
   */

  // alt,altrgr,ctrl ei kuulu rekisteriin. lähetetään kun tarvii koska käyttäjä painaa itse nappia, koska esim £ ei ole olemassa usb scancodessa (ainakaan suomi näppkselle)
  static bool capsLock = false;
  static bool numLock = true;
  static bool scrollLock = false;
  static bool mouseLock = false; // toggled by pausebreak

  if (scanCodeType == 6) {
    scanCodeType = 7; //set invalid scanode type to avoid triggering this function 8 times per pausebreak press (sends 8 bytes). the value will be reset by parsebuffer
    mouseLock = !mouseLock; 
  }

  if (mouseLock) {
    switch(scanCode) { //uses numpad mumbers for mouse
      case 0x75: // up arrow
        Mouse.move(0, -10, 0); //xVal, yPos, wheel
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x74: // right
        Mouse.move(10, 0, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x72: // down
        Mouse.move(0, 10, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x6B: // left
        Mouse.move(-10, 0, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse

      case 0x7D: // up right
        Mouse.move(10, -10, 0); //xVal, yPos, wheel
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x7A: // down right
        Mouse.move(10, 10, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x69: // down left
        Mouse.move(-10, 10, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x6c: // up left
        Mouse.move(-10, -10, 0);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse

      case 0x73:
        Mouse.click();
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
      case 0x70:
        Mouse.click(MOUSE_RIGHT);
        scanCode = 0; // clear scancode to void moving one tick in previous direction
        return; //return early, dont send arrowkeys when using as mouse
    }
  }

  Serial.println(mouseLock);

  // no shift no capslock. if multiple codes from usb-fin-scancodelist
  // produce same key, one with lowest value is used in this translator expect keys specified as numpad
  // which use corresponding value to send numpad not normal version of the key
  
  // keyboard library magically fixes capslock+shift to correct characters. if using other libraries consider using  const uint8_t scancodeToKey [4][256] type solution instead
  // also magically fixes numpad pageup etc to work with numlock
  const uint8_t scancodeToKey [256] {
    0x00, // 000 (0x00)
     202, // 001 (0x01) F1
    0x02, // 002 (0x02)
     198, // 003 (0x03) F5
     196, // 004 (0x04) F3
     194, // 005 (0x05) F1
     195, // 006 (0x06) F2
     205, // 007 (0x07) F12
    0x08, // 008 (0x08)
     203, // 009 (0x09) F10
     201, // 010 (0x0A) F8
     199, // 011 (0x0B) F6
     197, // 012 (0x0C) F4
     179, // 013 (0x0D) tab
     189, // 014 (0x0E) §
    0x0F, // 015 (0x0F)
    0x10, // 016 (0x10)
     130, // 017 (0x11) left alt
     129, // 018 (0x12) left shift
    0x13, // 019 (0x13)
     128, // 020 (0x14) left control
     113, // 021 (0x15) q
      49, // 022 (0x16) 1
    0x17, // 023 (0x17)
    0x18, // 024 (0x18)
    0x19, // 025 (0x19)
     122, // 026 (0x1A) z
     115, // 027 (0x1B) s
      97, // 028 (0x1C) a
     119, // 029 (0x1D) w
      50, // 030 (0x1E) 2
    0x1F, // 031 (0x1F)
    0x20, // 032 (0x20)
      99, // 033 (0x21) c
     120, // 034 (0x22) x
     100, // 035 (0x23) d
     101, // 036 (0x24) e
      52, // 037 (0x25) 4
      51, // 038 (0x26) 3
    0x27, // 039 (0x27)
    0x28, // 040 (0x28)
      32, // 041 (0x29) space
     118, // 042 (0x2A) v
     102, // 043 (0x2B) f
     116, // 044 (0x2C) t
     114, // 045 (0x2D) r
      53, // 046 (0x2E) 5
    0x2F, // 047 (0x2F)
    0x30, // 048 (0x30)
     110, // 049 (0x31) n
      98, // 050 (0x32) b
     104, // 051 (0x33) h
     103, // 052 (0x34) g
     121, // 053 (0x35) y
      54, // 054 (0x36) 6
    0x37, // 055 (0x37)
    0x38, // 056 (0x38)
    0x39, // 057 (0x39)
     109, // 058 (0x3A) m
     106, // 059 (0x3B) j
     117, // 060 (0x3C) u
      55, // 061 (0x3D) 7
      56, // 062 (0x3E) 8
    0x3F, // 063 (0x3F)
    0x40, // 064 (0x40)
      44, // 065 (0x41) ,
     107, // 066 (0x42) k
     105, // 067 (0x43) i
     111, // 068 (0x44) o
      48, // 069 (0x45) 0
      57, // 070 (0x46) 9
    0x47, // 071 (0x47)
    0x48, // 072 (0x48)
      46, // 073 (0x49) .
      47, // 074 (0x4A) -
     108, // 075 (0x4B) l
     187, // 076 (0x4C) ö
     112, // 077 (0x4D) p
      45, // 078 (0x4E) +
    0x4F, // 079 (0x4F)
    0x50, // 080 (0x50)
    0x51, // 081 (0x51)
     188, // 082 (0x52) ä
    0x53, // 083 (0x53)
      91, // 084 (0x54) å
      61, // 085 (0x55) ´
    0x56, // 086 (0x56)
    0x57, // 087 (0x57)
     193, // 088 (0x58) caps lock
     133, // 089 (0x59) right shift
     176, // 090 (0x5A) enter
      93, // 091 (0x5B) ¨
    0x5C, // 092 (0x5C)
      92, // 093 (0x5D) '
    0x5E, // 094 (0x5E)
    0x5F, // 095 (0x5F)
    0x60, // 096 (0x60)
     236, // 097 (0x61) <
    0x62, // 098 (0x62)
    0x63, // 099 (0x63)
    0x64, // 100 (0x64)
    0x65, // 101 (0x65)
     178, // 102 (0x66) backspace
    0x67, // 103 (0x67)
    0x68, // 104 (0x68)
     225, // 105 (0x69) numpad 1
    0x6A, // 106 (0x6A)
     228, // 107 (0x6B) numpad 4
     231, // 108 (0x6C) numpad 7
    0x6D, // 109 (0x6D)
    0x6E, // 110 (0x6E)
    0x6F, // 111 (0x6F)
     234, // 112 (0x70) numpad 0
     235, // 113 (0x71) numpad ,
     226, // 114 (0x72) numpad 2
     229, // 115 (0x73) numpad 5
     230, // 116 (0x74) numpad 6
     232, // 117 (0x75) numpad 8
     177, // 118 (0x76) esc
     219, // 119 (0x77) numlock
     204, // 120 (0x78) F11
     223, // 121 (0x79) numpad +
     227, // 122 (0x7A) numpad 3
     222, // 123 (0x7B) numpad -
     221, // 124 (0x7C) numpad *
     233, // 125 (0x7D) numpad 9
     207, // 126 (0x7E) scroll lock
    0x7F, // 127 (0x7F)
    0x80, // 128 (0x80)
    0x81, // 129 (0x81)
    0x82, // 130 (0x82)
     200, // 131 (0x83) F7
    0x84, // 132 (0x84)
    0x85, // 133 (0x85)
    0x86, // 134 (0x86)
    0x87, // 135 (0x87)
    0x88, // 136 (0x88)
    0x89, // 137 (0x89)
    0x8A, // 138 (0x8A)
    0x8B, // 139 (0x8B)
    0x8C, // 140 (0x8C)
    0x8D, // 141 (0x8D)
    0x8E, // 142 (0x8E)
    0x8F, // 143 (0x8F)
    0x90, // 144 (0x90)
    0x91, // 145 (0x91)
    0x92, // 146 (0x92)
    0x93, // 147 (0x93)
    0x94, // 148 (0x94)
    0x95, // 149 (0x95)
    0x96, // 150 (0x96)
    0x97, // 151 (0x97)
    0x98, // 152 (0x98)
    0x99, // 153 (0x99)
    0x9A, // 154 (0x9A)
    0x9B, // 155 (0x9B)
    0x9C, // 156 (0x9C)
    0x9D, // 157 (0x9D)
    0x9E, // 158 (0x9E)
    0x9F, // 159 (0x9F)
    0xA0, // 160 (0xA0)
    0xA1, // 161 (0xA1)
    0xA2, // 162 (0xA2)
    0xA3, // 163 (0xA3)
    0xA4, // 164 (0xA4)
    0xA5, // 165 (0xA5)
    0xA6, // 166 (0xA6)
    0xA7, // 167 (0xA7)
    0xA8, // 168 (0xA8)
    0xA9, // 169 (0xA9)
    0xAA, // 170 (0xAA)
    0xAB, // 171 (0xAB)
    0xAC, // 172 (0xAC)
    0xAD, // 173 (0xAD)
    0xAE, // 174 (0xAE)
    0xAF, // 175 (0xAF)
    0xB0, // 176 (0xB0)
    0xB1, // 177 (0xB1)
    0xB2, // 178 (0xB2)
    0xB3, // 179 (0xB3)
    0xB4, // 180 (0xB4)
    0xB5, // 181 (0xB5)
    0xB6, // 182 (0xB6)
    0xB7, // 183 (0xB7)
    0xB8, // 184 (0xB8)
    0xB9, // 185 (0xB9)
    0xBA, // 186 (0xBA)
    0xBB, // 187 (0xBB)
    0xBC, // 188 (0xBC)
    0xBD, // 189 (0xBD)
    0xBE, // 190 (0xBE)
    0xBF, // 191 (0xBF)
    0xC0, // 192 (0xC0)
    0xC1, // 193 (0xC1)
    0xC2, // 194 (0xC2)
    0xC3, // 195 (0xC3)
    0xC4, // 196 (0xC4)
    0xC5, // 197 (0xC5)
    0xC6, // 198 (0xC6)
    0xC7, // 199 (0xC7)
    0xC8, // 200 (0xC8)
    0xC9, // 201 (0xC9)
    0xCA, // 202 (0xCA)
    0xCB, // 203 (0xCB)
    0xCC, // 204 (0xCC)
    0xCD, // 205 (0xCD)
    0xCE, // 206 (0xCE)
    0xCF, // 207 (0xCF)
    0xD0, // 208 (0xD0)
    0xD1, // 209 (0xD1)
    0xD2, // 210 (0xD2)
    0xD3, // 211 (0xD3)
    0xD4, // 212 (0xD4)
    0xD5, // 213 (0xD5)
    0xD6, // 214 (0xD6)
    0xD7, // 215 (0xD7)
    0xD8, // 216 (0xD8)
    0xD9, // 217 (0xD9)
    0xDA, // 218 (0xDA)
    0xDB, // 219 (0xDB)
    0xDC, // 220 (0xDC)
    0xDD, // 221 (0xDD)
    0xDE, // 222 (0xDE)
    0xDF, // 223 (0xDF)
    0xE0, // 224 (0xE0)
    0xE1, // 225 (0xE1)
    0xE2, // 226 (0xE2)
    0xE3, // 227 (0xE3)
    0xE4, // 228 (0xE4)
    0xE5, // 229 (0xE5)
    0xE6, // 230 (0xE6)
    0xE7, // 231 (0xE7)
    0xE8, // 232 (0xE8)
    0xE9, // 233 (0xE9)
    0xEA, // 234 (0xEA)
    0xEB, // 235 (0xEB)
    0xEC, // 236 (0xEC)
    0xED, // 237 (0xED)
    0xEE, // 238 (0xEE)
    0xEF, // 239 (0xEF)
    0xF0, // 240 (0xF0)
    0xF1, // 241 (0xF1)
    0xF2, // 242 (0xF2)
    0xF3, // 243 (0xF3)
    0xF4, // 244 (0xF4)
    0xF5, // 245 (0xF5)
    0xF6, // 246 (0xF6)
    0xF7, // 247 (0xF7)
    0xF8, // 248 (0xF8)
    0xF9, // 249 (0xF9)
    0xFA, // 250 (0xFA)
    0xFB, // 251 (0xFB)
    0xFC, // 252 (0xFC)
    0xFD, // 253 (0xFD)
    0xFE, // 254 (0xFE)
    0xFF  // 255 (0xFF)
  };

  const uint8_t scancodeToKeySpecial [256] {
    0x00, // 000 (0x00)
    0x01, // 001 (0x01)
    0x02, // 002 (0x02)
    0x03, // 003 (0x03)
    0x04, // 004 (0x04)
    0x05, // 005 (0x05)
    0x06, // 006 (0x06)
    0x07, // 007 (0x07)
    0x08, // 008 (0x08)
    0x09, // 009 (0x09)
    0x0A, // 010 (0x0A)
    0x0B, // 011 (0x0B)
    0x0C, // 012 (0x0C)
    0x0D, // 013 (0x0D)
    0x0E, // 014 (0x0E)
    0x0F, // 015 (0x0F)
    0x10, // 016 (0x10)
     134, // 017 (0x11) alt gr
    0x12, // 018 (0x12)
    0x13, // 019 (0x13)
     132, // 020 (0x14) control act
    0x15, // 021 (0x15)
    0x16, // 022 (0x16)
    0x17, // 023 (0x17)
    0x18, // 024 (0x18)
    0x19, // 025 (0x19)
    0x1A, // 026 (0x1A)
    0x1B, // 027 (0x1B)
    0x1C, // 028 (0x1C)
    0x1D, // 029 (0x1D)
    0x1E, // 030 (0x1E)
    0x1F, // 031 (0x1F)
    0x20, // 032 (0x20)
    0x21, // 033 (0x21)
    0x22, // 034 (0x22)
    0x23, // 035 (0x23)
    0x24, // 036 (0x24)
    0x25, // 037 (0x25)
    0x26, // 038 (0x26)
    0x27, // 039 (0x27)
    0x28, // 040 (0x28)
    0x29, // 041 (0x29)
    0x2A, // 042 (0x2A)
    0x2B, // 043 (0x2B)
    0x2C, // 044 (0x2C)
    0x2D, // 045 (0x2D)
    0x2E, // 046 (0x2E)
    0x2F, // 047 (0x2F)
    0x30, // 048 (0x30)
    0x31, // 049 (0x31)
    0x32, // 050 (0x32)
    0x33, // 051 (0x33)
    0x34, // 052 (0x34)
    0x35, // 053 (0x35)
    0x36, // 054 (0x36)
    0x37, // 055 (0x37)
    0x38, // 056 (0x38)
    0x39, // 057 (0x39)
    0x3A, // 058 (0x3A)
    0x3B, // 059 (0x3B)
    0x3C, // 060 (0x3C)
    0x3D, // 061 (0x3D)
    0x3E, // 062 (0x3E)
    0x3F, // 063 (0x3F)
    0x40, // 064 (0x40)
    0x41, // 065 (0x41)
    0x42, // 066 (0x42)
    0x43, // 067 (0x43)
    0x44, // 068 (0x44)
    0x45, // 069 (0x45)
    0x46, // 070 (0x46)
    0x47, // 071 (0x47)
    0x48, // 072 (0x48)
    0x49, // 073 (0x49)
     220, // 074 (0x4A) numpad /
    0x4B, // 075 (0x4B)
    0x4C, // 076 (0x4C)
    0x4D, // 077 (0x4D)
    0x4E, // 078 (0x4E)
    0x4F, // 079 (0x4F)
    0x50, // 080 (0x50)
    0x51, // 081 (0x51)
    0x52, // 082 (0x52)
    0x53, // 083 (0x53)
    0x54, // 084 (0x54)
    0x55, // 085 (0x55)
    0x56, // 086 (0x56)
    0x57, // 087 (0x57)
    0x58, // 088 (0x58)
    0x59, // 089 (0x59)
     224, // 090 (0x5A) numpad enter
    0x5B, // 091 (0x5B)
    0x5C, // 092 (0x5C)
    0x5D, // 093 (0x5D)
    0x5E, // 094 (0x5E)
    0x5F, // 095 (0x5F)
    0x60, // 096 (0x60)
    0x61, // 097 (0x61)
    0x62, // 098 (0x62)
    0x63, // 099 (0x63)
    0x64, // 100 (0x64)
    0x65, // 101 (0x65)
    0x66, // 102 (0x66)
    0x67, // 103 (0x67)
    0x68, // 104 (0x68)
     213, // 105 (0x69) end
    0x6A, // 106 (0x6A)
     216, // 107 (0x6B) arrow left
     210, // 108 (0x6C) home
    0x6D, // 109 (0x6D)
    0x6E, // 110 (0x6E)
    0x6F, // 111 (0x6F)
     209, // 112 (0x70) insert
     212, // 113 (0x71) delete
     217, // 114 (0x72) arrow down
    0x73, // 115 (0x73)
     215, // 116 (0x74) arrow right
     218, // 117 (0x75) arrow up
    0x76, // 118 (0x76)
    0x77, // 119 (0x77)
    0x78, // 120 (0x78)
    0x79, // 121 (0x79)
     214, // 122 (0x7A) page down
    0x7B, // 123 (0x7B)
    0x7C, // 124 (0x7C)
     211, // 125 (0x7D) page up
    0x7E, // 126 (0x7E)
    0x7F, // 127 (0x7F)
    0x80, // 128 (0x80)
    0x81, // 129 (0x81)
    0x82, // 130 (0x82)
    0x83, // 131 (0x83)
    0x84, // 132 (0x84)
    0x85, // 133 (0x85)
    0x86, // 134 (0x86)
    0x87, // 135 (0x87)
    0x88, // 136 (0x88)
    0x89, // 137 (0x89)
    0x8A, // 138 (0x8A)
    0x8B, // 139 (0x8B)
    0x8C, // 140 (0x8C)
    0x8D, // 141 (0x8D)
    0x8E, // 142 (0x8E)
    0x8F, // 143 (0x8F)
    0x90, // 144 (0x90)
    0x91, // 145 (0x91)
    0x92, // 146 (0x92)
    0x93, // 147 (0x93)
    0x94, // 148 (0x94)
    0x95, // 149 (0x95)
    0x96, // 150 (0x96)
    0x97, // 151 (0x97)
    0x98, // 152 (0x98)
    0x99, // 153 (0x99)
    0x9A, // 154 (0x9A)
    0x9B, // 155 (0x9B)
    0x9C, // 156 (0x9C)
    0x9D, // 157 (0x9D)
    0x9E, // 158 (0x9E)
    0x9F, // 159 (0x9F)
    0xA0, // 160 (0xA0)
    0xA1, // 161 (0xA1)
    0xA2, // 162 (0xA2)
    0xA3, // 163 (0xA3)
    0xA4, // 164 (0xA4)
    0xA5, // 165 (0xA5)
    0xA6, // 166 (0xA6)
    0xA7, // 167 (0xA7)
    0xA8, // 168 (0xA8)
    0xA9, // 169 (0xA9)
    0xAA, // 170 (0xAA)
    0xAB, // 171 (0xAB)
    0xAC, // 172 (0xAC)
    0xAD, // 173 (0xAD)
    0xAE, // 174 (0xAE)
    0xAF, // 175 (0xAF)
    0xB0, // 176 (0xB0)
    0xB1, // 177 (0xB1)
    0xB2, // 178 (0xB2)
    0xB3, // 179 (0xB3)
    0xB4, // 180 (0xB4)
    0xB5, // 181 (0xB5)
    0xB6, // 182 (0xB6)
    0xB7, // 183 (0xB7)
    0xB8, // 184 (0xB8)
    0xB9, // 185 (0xB9)
    0xBA, // 186 (0xBA)
    0xBB, // 187 (0xBB)
    0xBC, // 188 (0xBC)
    0xBD, // 189 (0xBD)
    0xBE, // 190 (0xBE)
    0xBF, // 191 (0xBF)
    0xC0, // 192 (0xC0)
    0xC1, // 193 (0xC1)
    0xC2, // 194 (0xC2)
    0xC3, // 195 (0xC3)
    0xC4, // 196 (0xC4)
    0xC5, // 197 (0xC5)
    0xC6, // 198 (0xC6)
    0xC7, // 199 (0xC7)
    0xC8, // 200 (0xC8)
    0xC9, // 201 (0xC9)
    0xCA, // 202 (0xCA)
    0xCB, // 203 (0xCB)
    0xCC, // 204 (0xCC)
    0xCD, // 205 (0xCD)
    0xCE, // 206 (0xCE)
    0xCF, // 207 (0xCF)
    0xD0, // 208 (0xD0)
    0xD1, // 209 (0xD1)
    0xD2, // 210 (0xD2)
    0xD3, // 211 (0xD3)
    0xD4, // 212 (0xD4)
    0xD5, // 213 (0xD5)
    0xD6, // 214 (0xD6)
    0xD7, // 215 (0xD7)
    0xD8, // 216 (0xD8)
    0xD9, // 217 (0xD9)
    0xDA, // 218 (0xDA)
    0xDB, // 219 (0xDB)
    0xDC, // 220 (0xDC)
    0xDD, // 221 (0xDD)
    0xDE, // 222 (0xDE)
    0xDF, // 223 (0xDF)
    0xE0, // 224 (0xE0)
    0xE1, // 225 (0xE1)
    0xE2, // 226 (0xE2)
    0xE3, // 227 (0xE3)
    0xE4, // 228 (0xE4)
    0xE5, // 229 (0xE5)
    0xE6, // 230 (0xE6)
    0xE7, // 231 (0xE7)
    0xE8, // 232 (0xE8)
    0xE9, // 233 (0xE9)
    0xEA, // 234 (0xEA)
    0xEB, // 235 (0xEB)
    0xEC, // 236 (0xEC)
    0xED, // 237 (0xED)
    0xEE, // 238 (0xEE)
    0xEF, // 239 (0xEF)
    0xF0, // 240 (0xF0)
    0xF1, // 241 (0xF1)
    0xF2, // 242 (0xF2)
    0xF3, // 243 (0xF3)
    0xF4, // 244 (0xF4)
    0xF5, // 245 (0xF5)
    0xF6, // 246 (0xF6)
    0xF7, // 247 (0xF7)
    0xF8, // 248 (0xF8)
    0xF9, // 249 (0xF9)
    0xFA, // 250 (0xFA)
    0xFB, // 251 (0xFB)
    0xFC, // 252 (0xFC)
    0xFD, // 253 (0xFD)
    0xFE, // 254 (0xFE)
    0xFF  // 255 (0xFF)
  };

  const uint8_t scancodeToKeyExtraSpecial [256] { //numlock on
    0x00, // 000 (0x00)
    0x01, // 001 (0x01)
    0x02, // 002 (0x02)
    0x03, // 003 (0x03)
    0x04, // 004 (0x04)
    0x05, // 005 (0x05)
    0x06, // 006 (0x06)
    0x07, // 007 (0x07)
    0x08, // 008 (0x08)
    0x09, // 009 (0x09)
    0x0A, // 010 (0x0A)
    0x0B, // 011 (0x0B)
    0x0C, // 012 (0x0C)
    0x0D, // 013 (0x0D)
    0x0E, // 014 (0x0E)
    0x0F, // 015 (0x0F)
    0x10, // 016 (0x10)
    0x11, // 017 (0x11)
    0x12, // 018 (0x12)
    0x13, // 019 (0x13)
    0x14, // 020 (0x14)
    0x15, // 021 (0x15)
    0x16, // 022 (0x16)
    0x17, // 023 (0x17)
    0x18, // 024 (0x18)
    0x19, // 025 (0x19)
    0x1A, // 026 (0x1A)
    0x1B, // 027 (0x1B)
    0x1C, // 028 (0x1C)
    0x1D, // 029 (0x1D)
    0x1E, // 030 (0x1E)
    0x1F, // 031 (0x1F)
    0x20, // 032 (0x20)
    0x21, // 033 (0x21)
    0x22, // 034 (0x22)
    0x23, // 035 (0x23)
    0x24, // 036 (0x24)
    0x25, // 037 (0x25)
    0x26, // 038 (0x26)
    0x27, // 039 (0x27)
    0x28, // 040 (0x28)
    0x29, // 041 (0x29)
    0x2A, // 042 (0x2A)
    0x2B, // 043 (0x2B)
    0x2C, // 044 (0x2C)
    0x2D, // 045 (0x2D)
    0x2E, // 046 (0x2E)
    0x2F, // 047 (0x2F)
    0x30, // 048 (0x30)
    0x31, // 049 (0x31)
    0x32, // 050 (0x32)
    0x33, // 051 (0x33)
    0x34, // 052 (0x34)
    0x35, // 053 (0x35)
    0x36, // 054 (0x36)
    0x37, // 055 (0x37)
    0x38, // 056 (0x38)
    0x39, // 057 (0x39)
    0x3A, // 058 (0x3A)
    0x3B, // 059 (0x3B)
    0x3C, // 060 (0x3C)
    0x3D, // 061 (0x3D)
    0x3E, // 062 (0x3E)
    0x3F, // 063 (0x3F)
    0x40, // 064 (0x40)
    0x41, // 065 (0x41)
    0x42, // 066 (0x42)
    0x43, // 067 (0x43)
    0x44, // 068 (0x44)
    0x45, // 069 (0x45)
    0x46, // 070 (0x46)
    0x47, // 071 (0x47)
    0x48, // 072 (0x48)
    0x49, // 073 (0x49)
    0x4A, // 074 (0x4A)
    0x4B, // 075 (0x4B)
    0x4C, // 076 (0x4C)
    0x4D, // 077 (0x4D)
    0x4E, // 078 (0x4E)
    0x4F, // 079 (0x4F)
    0x50, // 080 (0x50)
    0x51, // 081 (0x51)
    0x52, // 082 (0x52)
    0x53, // 083 (0x53)
    0x54, // 084 (0x54)
    0x55, // 085 (0x55)
    0x56, // 086 (0x56)
    0x57, // 087 (0x57)
    0x58, // 088 (0x58)
    0x59, // 089 (0x59)
    0x5A, // 090 (0x5A)
    0x5B, // 091 (0x5B)
    0x5C, // 092 (0x5C)
    0x5D, // 093 (0x5D)
    0x5E, // 094 (0x5E)
    0x5F, // 095 (0x5F)
    0x60, // 096 (0x60)
    0x61, // 097 (0x61)
    0x62, // 098 (0x62)
    0x63, // 099 (0x63)
    0x64, // 100 (0x64)
    0x65, // 101 (0x65)
    0x66, // 102 (0x66)
    0x67, // 103 (0x67)
    0x68, // 104 (0x68)
     213, // 105 (0x69) end
    0x6A, // 106 (0x6A)
     216, // 107 (0x6B) arrow left
     210, // 108 (0x6C) home
    0x6D, // 109 (0x6D)
    0x6E, // 110 (0x6E)
    0x6F, // 111 (0x6F)
     131, // 112 (0x70) insert 209. replaced with win key
     237, // 113 (0x71) delete 212. replaced with menu keu
     217, // 114 (0x72) arrow down
    0x73, // 115 (0x73)
     215, // 116 (0x74) arrow right
     218, // 117 (0x75) arrow up
    0x76, // 118 (0x76)
    0x77, // 119 (0x77)
    0x78, // 120 (0x78)
    0x79, // 121 (0x79)
     214, // 122 (0x7A) page down
    0x7B, // 123 (0x7B)
     206, // 124 (0x7C) printscreen
     211, // 125 (0x7D) page up
    0x7E, // 126 (0x7E)
    0x7F, // 127 (0x7F)
    0x80, // 128 (0x80)
    0x81, // 129 (0x81)
    0x82, // 130 (0x82)
    0x83, // 131 (0x83)
    0x84, // 132 (0x84)
    0x85, // 133 (0x85)
    0x86, // 134 (0x86)
    0x87, // 135 (0x87)
    0x88, // 136 (0x88)
    0x89, // 137 (0x89)
    0x8A, // 138 (0x8A)
    0x8B, // 139 (0x8B)
    0x8C, // 140 (0x8C)
    0x8D, // 141 (0x8D)
    0x8E, // 142 (0x8E)
    0x8F, // 143 (0x8F)
    0x90, // 144 (0x90)
    0x91, // 145 (0x91)
    0x92, // 146 (0x92)
    0x93, // 147 (0x93)
    0x94, // 148 (0x94)
    0x95, // 149 (0x95)
    0x96, // 150 (0x96)
    0x97, // 151 (0x97)
    0x98, // 152 (0x98)
    0x99, // 153 (0x99)
    0x9A, // 154 (0x9A)
    0x9B, // 155 (0x9B)
    0x9C, // 156 (0x9C)
    0x9D, // 157 (0x9D)
    0x9E, // 158 (0x9E)
    0x9F, // 159 (0x9F)
    0xA0, // 160 (0xA0)
    0xA1, // 161 (0xA1)
    0xA2, // 162 (0xA2)
    0xA3, // 163 (0xA3)
    0xA4, // 164 (0xA4)
    0xA5, // 165 (0xA5)
    0xA6, // 166 (0xA6)
    0xA7, // 167 (0xA7)
    0xA8, // 168 (0xA8)
    0xA9, // 169 (0xA9)
    0xAA, // 170 (0xAA)
    0xAB, // 171 (0xAB)
    0xAC, // 172 (0xAC)
    0xAD, // 173 (0xAD)
    0xAE, // 174 (0xAE)
    0xAF, // 175 (0xAF)
    0xB0, // 176 (0xB0)
    0xB1, // 177 (0xB1)
    0xB2, // 178 (0xB2)
    0xB3, // 179 (0xB3)
    0xB4, // 180 (0xB4)
    0xB5, // 181 (0xB5)
    0xB6, // 182 (0xB6)
    0xB7, // 183 (0xB7)
    0xB8, // 184 (0xB8)
    0xB9, // 185 (0xB9)
    0xBA, // 186 (0xBA)
    0xBB, // 187 (0xBB)
    0xBC, // 188 (0xBC)
    0xBD, // 189 (0xBD)
    0xBE, // 190 (0xBE)
    0xBF, // 191 (0xBF)
    0xC0, // 192 (0xC0)
    0xC1, // 193 (0xC1)
    0xC2, // 194 (0xC2)
    0xC3, // 195 (0xC3)
    0xC4, // 196 (0xC4)
    0xC5, // 197 (0xC5)
    0xC6, // 198 (0xC6)
    0xC7, // 199 (0xC7)
    0xC8, // 200 (0xC8)
    0xC9, // 201 (0xC9)
    0xCA, // 202 (0xCA)
    0xCB, // 203 (0xCB)
    0xCC, // 204 (0xCC)
    0xCD, // 205 (0xCD)
    0xCE, // 206 (0xCE)
    0xCF, // 207 (0xCF)
    0xD0, // 208 (0xD0)
    0xD1, // 209 (0xD1)
    0xD2, // 210 (0xD2)
    0xD3, // 211 (0xD3)
    0xD4, // 212 (0xD4)
    0xD5, // 213 (0xD5)
    0xD6, // 214 (0xD6)
    0xD7, // 215 (0xD7)
    0xD8, // 216 (0xD8)
    0xD9, // 217 (0xD9)
    0xDA, // 218 (0xDA)
    0xDB, // 219 (0xDB)
    0xDC, // 220 (0xDC)
    0xDD, // 221 (0xDD)
    0xDE, // 222 (0xDE)
    0xDF, // 223 (0xDF)
    0xE0, // 224 (0xE0)
    0xE1, // 225 (0xE1)
    0xE2, // 226 (0xE2)
    0xE3, // 227 (0xE3)
    0xE4, // 228 (0xE4)
    0xE5, // 229 (0xE5)
    0xE6, // 230 (0xE6)
    0xE7, // 231 (0xE7)
    0xE8, // 232 (0xE8)
    0xE9, // 233 (0xE9)
    0xEA, // 234 (0xEA)
    0xEB, // 235 (0xEB)
    0xEC, // 236 (0xEC)
    0xED, // 237 (0xED)
    0xEE, // 238 (0xEE)
    0xEF, // 239 (0xEF)
    0xF0, // 240 (0xF0)
    0xF1, // 241 (0xF1)
    0xF2, // 242 (0xF2)
    0xF3, // 243 (0xF3)
    0xF4, // 244 (0xF4)
    0xF5, // 245 (0xF5)
    0xF6, // 246 (0xF6)
    0xF7, // 247 (0xF7)
    0xF8, // 248 (0xF8)
    0xF9, // 249 (0xF9)
    0xFA, // 250 (0xFA)
    0xFB, // 251 (0xFB)
    0xFC, // 252 (0xFC)
    0xFD, // 253 (0xFD)
    0xFE, // 254 (0xFE)
    0xFF  // 255 (0xFF)
  };

  switch(scanCodeType) {
    case 0:      // make
      Keyboard.press(scancodeToKey[scanCode]);
      break;
    case 1:      // break
      Keyboard.release(scancodeToKey[scanCode]);
      break;
    case 2:      // specialmake
      Keyboard.press(scancodeToKeySpecial[scanCode]);
      break;
    case 3:      // specialbreak
      Keyboard.release(scancodeToKeySpecial[scanCode]);
      break;
    case 4:      // extraspecialmake
      Keyboard.press(scancodeToKeyExtraSpecial[scanCode]);
      break;
    case 5:      // extraspecialbreak
      Keyboard.release(scancodeToKeyExtraSpecial[scanCode]);
      break;
    case 6:      // pausebreak
      Keyboard.write(208); //not using table here. pausebreak doesnt need 256 sized array alone.
  }
}




void parseNewBuffer(void) {
  /**
   * this function uses latest byte from keyboard in newBuffer with previous
   * byte in oldBuffer to parse keypresses and releases. oldBuffer values are
   * updated by this function and newBuffer is emptied after parsing is complete
   * 
   * keyboard sends 3 types of presses/releases
   * -  3 byte, 1 for make code and 2 for break code. used by wasd and 123 etc
   * -  5 byte, 2 for make code and 3 for break code. used by alt_gr etc
   * - 10 byte, 4 for make code and 6 for break code. used by printscreen etc
   * 
   * 1 byte make codes are self explanatory. 2 byte make codes are prefixed with
   * 0xE0 byte. later called "special". 4 byte make codes are prefixed with
   * special-leftshift-special (0xE0 0x12 0xE0) later called "extraspecial"
   * 
   * following block contains "data" codes and their respective keys. please
   * note that all make and break codes are constructed from "data" codes by
   * adding modifier bytes like special, extraspecial and break. all 3 different
   * key types are presented following next pattern which has make set before #
   * and break set after it: [modifier_x data # modifier_x modifier_y data]
   ******************************[ 3 byte data set ]****************************
   *
   * [data # break data]
   * 0x15 Q    0x2B F    0x3A M              0x0E §    0x05 F1     0x55 ´
   * 0x1D W    0x34 G    0x66 BACKSPACE      0x4E +    0x06 F2     0x41 ,  
   * 0x24 E    0x33 H    0x5A ENTER          0x4A -    0x04 F3     0x49 . 
   * 0x2D R    0x3B J    0x0D TAB            0x45 0    0x0C F4     0x70 NUMPAD_0
   * 0x2C T    0x42 K    0x59 R_SHIFT        0x16 1    0x03 F5     0x69 NUMPAD_1
   * 0x35 Y    0x4B L    0x11 L_ALT          0x1E 2    0x0B F6     0x72 NUMPAD_2
   * 0x3C U    0x4C Ö    0x79 NUMPAD_+       0x26 3    0x83 F7     0x7A NUMPAD_3
   * 0x43 I    0x52 Ä    0x7C NUMPAD_*       0x25 4    0x0A F8     0x6B NUMPAD_4
   * 0x44 O    0x1A Z    0x7B NUMPAD_-       0x2E 5    0x01 F9     0x73 NUMPAD_5
   * 0x4D P    0x22 X    0x14 L_CTRL         0x36 6    0x09 F10    0x74 NUMPAD_6
   * 0x54 Å    0x21 C    0x58 CAPS_LOCK      0x3D 7    0x78 F11    0x6C NUMPAD_7
   * 0x1C A    0x2A V    0x77 NUM_LOCK       0x3E 8    0x07 F12    0x75 NUMPAD_8
   * 0x1B S    0x32 B    0x7E SCROLL_LOCK    0x46 9    0x5D '      0x7D NUMPAD_9
   * 0x23 D    0x31 N    0x12 L_SHIFT        0x5B ¨    0x61 <      0x29 SPACE
   *                                                   0x76 ESC    0x71 NUMPAD_,
   * 
   ******************************[ 5 byte data set ]****************************
   *
   * [special data # special break data]
   * 0x75 ARROW_UP       0x70 INSERT    0x7D PAGE_UP      0x4A NUMPAD_/
   * 0x74 ARROW_RIGHT    0x71 DELETE    0x7A PAGE_DOWN    0x5A NUMPAD_ENTER
   * 0x72 ARROW_DOWN     0x6C HOME      0x11 ALT_GR
   * 0x6B ARROW_LEFT     0x69 END       0x14 CTRL_ACT
   * 
   *****************************[ 10 byte data set ]****************************
   *
   * [special shift special data # special break data special break shift]
   * ( !! THESE ONLY OCCUR IF NUMLOCK IS TOGGLED. EXCEPT PRINTSCREEN ALWAYS !! )
   * 0x75 ARROW_UP       0x70 INSERT    0x7D PAGE_UP
   * 0x74 ARROW_RIGHT    0x71 DELETE    0x7A PAGE_DOWN
   * 0x72 ARROW_DOWN     0x6C HOME      0x7C PRINTSCREEN
   * 0x6B ARROW_LEFT     0x69 END
   * 
   **************************[ modifiers + pause/break ]************************
   *
   *( !! ONLY SENT TOGETHER WITH "DATA" BYTES. PAUSEBREAK FITS NO CATEGORY !! )
   * break           0xF0
   * special         0xE0
   * secial shift    0xE0 0x12
   * pausebreak      0xE1 0x14 0x77 0xE1 0xF0 0x14 0xF0 0x77 (HAS NO BREAK CODE)
   */

  static uint8_t pausebreakCounter = 0;   // escapes function on pasebreak
  static uint8_t oldBuffer = 0;           // previous newBuffer value
  static uint8_t specialShiftBuffer1 = 0; // buffer1 extraspecial key (2KRO)
  static uint8_t specialShiftBuffer2 = 0; // buffer1 extraspecial key (2KRO)
  
  static bool specialShiftFlag = false;   // 0xE0 0x12 received. for 10 byte key
  static bool specialFlag = false;        // 0xE0 received. for 5 byte key
  static bool breakFlag = false;          // break for 3,5,10 byte keys

  if (newBuffer == 0xE1) { // catch the 7 byte pausebreak mystery key without break codes
    if (pausebreakCounter == 0) {
       Serial.println("MAKE+BREAK: PAUSEBREAK\n");
      scanCodeType = 6; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
      scanCode = 0xE1; // latest code to make or break
    }
    pausebreakCounter = 4; // pausebreak 0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77. this counter gets to 4 twice to ignore all bytes
    newBuffer = 0; // always reset before return!
    return; //early return. skip rest of the function call.
  }
  if (pausebreakCounter != 0) { //pausebreak received on some previous function call. ignore rest of pausebreaks data
    pausebreakCounter--;
    newBuffer = 0; // always reset before return!
    return;
  }

  if (oldBuffer == 0xE0) specialFlag = 1; // 0xE0 in newBuffer on previous call
  if (oldBuffer == 0xF0) breakFlag = 1;   // 0xF0 in newBuffer on previous call

  oldBuffer = newBuffer; // rotate buffers


  // make block starts here. no breakflag set from oldbuffer and no special or break byte in newbuffer -> make
  if ((not breakFlag) and (newBuffer != 0xE0) and (newBuffer != 0xF0)) { //if no flags true must be make code
    if (specialFlag) { // if special flag set on previous function call
      specialFlag = false; // reset flag
      if (specialShiftFlag) { // check if flag is set BEFORE setting it so its not triggered on same "round"
        if (specialShiftBuffer1 == 0) { //check that buffer not already in use. keyboard is 2KRO so only 2 buffers
          specialShiftBuffer1 = newBuffer; // fill 1st buffer dah
        }
        else {
          specialShiftBuffer2 = newBuffer; // and 2nd if first one in use already. 2KRO allowed
        }
        Serial.print("EXTRASPECIALMAKE:"); // also actually make the key after buffering
        DEBUG_HEXPRINT(newBuffer);
        Serial.println("");
        scanCodeType = 4; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
        scanCode = newBuffer; // latest code to make or break
      }
      if (newBuffer == 0x12) { //special shifted key incoming. set flag for _NEXT_ round (aka after checking for it)
        specialShiftFlag = true;
      }
      else if ((newBuffer != specialShiftBuffer1) and (newBuffer != specialShiftBuffer2)){ // if no extraspecialmake when specialmake. also avoid printing extraspecialmake again
        Serial.print("SPECIALMAKE:");
        DEBUG_HEXPRINT(newBuffer);
        Serial.println("");
        scanCodeType = 2; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
        scanCode = newBuffer; // latest code to make or break
      }
    }
    else { // if no extraspecialmake or special make then ele just normal make
      Serial.print("MAKE:");
      DEBUG_HEXPRINT(newBuffer);
      Serial.println("");
      scanCodeType = 0; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
      scanCode = newBuffer; // latest code to make or break
    }
  } // make block ends here

  // break block starts here. breakflag set from oldbuffer
  if (breakFlag) { // if breakflag was set on previus function call
    breakFlag = false; // reset break flag
    if (specialFlag) { //and check if break should be special (or extraspecial) or just normal break
      specialFlag = false; // reset flag
      if (newBuffer == specialShiftBuffer2) { //if incoming byte is stored already in buffer it must be correct to extraspecialbreak. also note that we check bufer 2 first at it was written last
        Serial.print("EXTRASPECIALBREAK:");
        DEBUG_HEXPRINT(newBuffer);
        Serial.println("");
        specialShiftBuffer2 = 0;
        scanCodeType = 5; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
        scanCode = newBuffer; // latest code to make or break
      }
      else if (newBuffer == specialShiftBuffer1) { //and if nothing was in buffer2 then check for buffr1 to extraspecialbreak
        Serial.print("EXTRASPECIALBREAK:");
        DEBUG_HEXPRINT(newBuffer);
        Serial.println("");
        specialShiftBuffer1 = 0;
        scanCodeType = 5; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
        scanCode = newBuffer; // latest code to make or break
      }
      else { 
        if (newBuffer == 0x12) { //and if nothing was to be extrasecialprinted check if extralspecialshift should be turned of to allow for keys to be repeated
          specialShiftFlag = false; 
        }
        else {
          Serial.print("SPECIALBREAK:"); // if no extraspecialbreak and no flag to turn of then just specialbreak
          DEBUG_HEXPRINT(newBuffer);
          Serial.println("");
          scanCodeType = 3; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
          scanCode = newBuffer; // latest code to make or break
        }
      }
    }
    else { //if there was no special flag at all just normal break
      Serial.print("BREAK:");
      DEBUG_HEXPRINT(newBuffer);
      Serial.println("");
      scanCodeType = 1; // 0-make. 1-break. 2-specialmake. 3-specialbreak. 4-extraspecialmake. 5-extraspecialbreak. 6-pausebreak
      scanCode = newBuffer; // latest code to make or break
    }
  }

  newBuffer = 0; // empty buffer so mainloop knows buffer has been parsed
}


void keyboardRead(void) {
  // reads what keyboard is sending bit by bit. accumulates it in dataIn and finally after checking for errors
  // writes complete bytes to readBuffer
  if (reset+2 < millis()){     // reset. note: millis only increments outside of interrupt handler
    dataIn = 0;
    bitCounter = 0;
  }

  bitWrite(dataIn, bitCounter, digitalRead(dataPin));
  bitCounter ++;

  if (bitCounter == 11) { // actually handle the completed byte here. atm only printing 
    /*
    | true value meanings of dataIn -----EPM::::::LB
    | - mask    mask to print leading zeroes
    | B begin   start bit
    | L LSB     read least significant bit
    | : data    read bit
    | M MSB     read most significant bit
    | P parity  parity bit
    | E end     stop bit
    */
   
    //Serial.print("ISR: ");// DEBUG
    //DEBUG_HEXPRINT((dataIn >> 1) & 0xff);// DEBUG

    if ((dataIn & 0x401) != 0x400) {   // check with bitmask if either start or stop fails and set error flag
      errorFlag = true;
    }
    else { // compute parity to see if actual data was mangled
      uint8_t parity = (dataIn & 0x1fe ) >> 1;
      parity ^= (parity >> 4);
      parity ^= (parity >> 2);
      parity ^= (parity >> 1);
      parity = ~parity & 1;
      if (parity != ((dataIn & 0x200) >> 9)) { // compare parity and calculated paritY
        errorFlag = true;
      }
    }
    newBuffer = (dataIn >> 1) & 0xff;
    dataIn = 0;
    bitCounter = 0;  
  }
  else { //bitcounter wasnt 11
    reset = millis(); // take note of time to see if random bit or something else than 11 bit input was sent from keyboard. used at the start of ISR to reset buffer
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(dataPin, INPUT_PULLUP);
  pinMode(irqPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(irqPin), keyboardRead, FALLING);
  //BootKeyboard.begin(); // nicohoods keyboard library
  Keyboard.begin(); //  keyboard library
  Mouse.begin(); //  keyboard library
}


void loop() {
  if (errorFlag){
    Serial.println("ERROR FLAG RAISED!");
    Serial.println("waiting 5s");
    delay(5000);
    errorFlag = false;
  }

  // busy waiting in mainloop for ISR to fill readBuffer
  if (newBuffer) {
    parseNewBuffer();
    simulateKey();
  }
}



/*
XOR operation (denoted *) can be thought of as parity check of two bits. this function uses it to check 8 bits in the following way
for 2 bits: 1*2
for 4 bits: (1*2)*(3*4)
for 8 bits: ((1*2)*(3*4))*((5*6)*(7*8))

logic proof with 8 bits abcdefgh. XOR operation is denoted with two adjacent characters.
for example bd(c ae) equals (b*d)*(c*(a*e)) with * denoting XOR here.
the table shows logical expressions for 8 bits when the function runs

+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------+
|               a |               b |               c |               d |               e |               f |               g |               h | x = initial value |
|               a |               b |               c |               d |              ae |              bf |              cg |              dh | x ^= (x >> 4)     |
|               a |               b |              ac |              bd |            c ae |            d bf |           ae cg |           bf dh | x ^= (x >> 2)     |
|               a |              ab |            b ac |           ac bd |        bd(c ae) |     c(ae) d(bf) |   d(bf) (ae cg) | (ae cg) (bf dh) | x ^= (x >> 1)     |
|               0 |               0 |               0 |               0 |               0 |               0 |               0 | (ae cg) (bf dh) | x = x & 1         |
+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------+
*/
