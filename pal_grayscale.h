#pragma once
#include "i2s_composite.hpp"

// 1Vp-p = 140*IRE
const float IRE = 1.0 / 140.0;

// Timings: http://www.batsocks.co.uk/readme/video_timing.htm
// http://martin.hinner.info/vga/pal.html
// Also interesting: https://wiki.nesdev.com/w/index.php/NTSC_video

const float imageAspect = 4./3.;

class CompositeOutput
{
  public:
  int samplesLine;
  int samplesHSync;
  int samplesBackPorch;
  int samplesFrontPorch;
  int samplesActive;
  int samplesBlackLeft;
  int samplesBlackRight;

  int samplesShortSync;
  int samplesBroadSync;

  char levelSync;
  char levelBlank;
  char levelBlack;
  char levelWhite;

  int width;
  int height;

  int linesBlackTop;
  int linesBlackBottom;

  float pixelAspect;
    
  unsigned short *line;

  CompositeOutput(int w, int h, double Vcc = 3.3)
  {
    // Timings, see http://www.batsocks.co.uk/readme/video_timing.htm
    const float lineMicros = 64;
    const float hSyncMicros = 4.7;
    const float backPorchMicros = 5.7;
    const float frontPorchMicros = 1.65;
    const float shortSyncMicros = 2.35;
    const float broadSyncMicros = (lineMicros / 2) - 4.7;
    // Levels, see https://www.maximintegrated.com/en/design/technical-documents/tutorials/7/734.html
    const float syncVolts = -40.0 * IRE;
    const float blankVolts = 0.0 * IRE;
    const float blackVolts = 7.5 * IRE;
    const float whiteVolts = 100.0 * IRE;
    // Line count
    const int lines = 625;
    const int verticalBlankingLines = 51;
  
    const double samplesPerMicro = 160.0 / 3.0 / 2.0 / 2.0;
    Serial.printf("samplesPerMicro: %lf\r\n", samplesPerMicro);
    Serial.printf("sample ms: %lf\r\n", 1.0 / samplesPerMicro);
    // Short Sync pulse
    samplesShortSync = samplesPerMicro * shortSyncMicros + 0.5;
    Serial.printf("samplesShortSync: %d\r\n", samplesShortSync);
    // Broad Sync pulse
    samplesBroadSync = samplesPerMicro * broadSyncMicros + 0.5;
    Serial.printf("samplesBroadSync: %d\r\n", samplesBroadSync);
    // Scanline
    samplesLine = (int)(samplesPerMicro * lineMicros + 1.5) & ~1;
    Serial.printf("samplesLine: %d\r\n", samplesLine);
    // Horizontal Sync
    samplesHSync = samplesPerMicro * hSyncMicros + 0.5;
    Serial.printf("samplesHSync: %d\r\n", samplesHSync);
    // Back Porch
    samplesBackPorch = samplesPerMicro * backPorchMicros + 0.5;
    Serial.printf("samplesBackPorch: %d\r\n", samplesBackPorch);
    // Front Porch
    samplesFrontPorch = samplesPerMicro * frontPorchMicros + 0.5;
    Serial.printf("samplesFrontPorch: %d\r\n", samplesFrontPorch);
    // Picture Data
    samplesActive = samplesLine - samplesHSync - samplesBackPorch - samplesFrontPorch;
    Serial.printf("samplesActive: %d\r\n", samplesActive);

    int linesActive = (lines - verticalBlankingLines);
    Serial.printf("linesActive: %d\r\n", linesActive);

    width = w < samplesActive ? w : samplesActive;
    height = h < linesActive ? h : linesActive;
    Serial.printf("Composite resolution: %dx%d\r\n", width, height);

    // Vertical centering
    int blackLines = (linesActive - height) / 2;
    linesBlackTop = blackLines / 2; // TODO: +1 is compensation
    Serial.printf("linesBlackTop: %d\r\n", linesBlackTop);
    linesBlackBottom = blackLines - linesBlackTop;
    Serial.printf("linesBlackBottom: %d\r\n", linesBlackBottom);

    // horizontal centering
    samplesBlackLeft = (samplesActive - width) / 2; // TODO: +7 is compensation
    Serial.printf("columnsBlackLeft: %d\r\n", samplesBlackLeft);
    samplesBlackRight = samplesActive - width - samplesBlackLeft;
    Serial.printf("columnsBlackRight: %d\r\n", samplesBlackRight);

    Serial.printf("frameStart: %d\r\n", samplesHSync + samplesBackPorch + samplesBlackLeft);

    double dacPerVolt = 255.0 / Vcc;
    levelSync = 0;
    Serial.printf("levelSync %d\r\n", levelSync);
    levelBlank = (blankVolts - syncVolts) * dacPerVolt + 0.5;
    Serial.printf("levelBlank %d\r\n", levelBlank);
    levelBlack = (blackVolts - syncVolts) * dacPerVolt + 0.5;
    Serial.printf("levelBlack %d\r\n", levelBlack);
    levelWhite = (whiteVolts - syncVolts) * dacPerVolt + 0.5;
    Serial.printf("levelWhite %d\r\n", levelWhite);

    pixelAspect = (float(samplesActive) / height) / imageAspect;
    Serial.printf("Composite res:(%dx%d)\r\n", width, height);

    line = (unsigned short*)malloc(sizeof(unsigned short) * samplesLine);
    i2s_composite.init(samplesLine);
  }

  void sendLine()
  {
    i2s_composite.write_line(line);
  }

  inline void fillValues(int &i, unsigned char value, int count)
  {
    for(int j = 0; j < count; j++)
      line[i++^1] = value << 8;
  }

  // One scanLine
  void fillLine(unsigned char *pixels, int fromPixel)
  {
    int i = 0;
    // HSync
    fillValues(i, levelSync, samplesHSync);
    // Back Porch
    fillValues(i, levelBlank, samplesBackPorch);
    // Black left (image centering)
    fillValues(i, levelBlack, samplesBlackLeft);
    // Picture Data
    for(int x = 0; x < width / 2; x++)
    {
      unsigned short pix = (levelBlack + pixels[fromPixel + x]) << 8;
      line[i++^1] = pix;
      line[i++^1] = pix;
    }
    // Black right (image centering)
    fillValues(i, levelBlack, samplesBlackRight);
    // Front Porch
    fillValues(i, levelBlank, samplesFrontPorch);
  }

  // Half a line of BroadSync
  void halfBroadSync(int &i)
  {
    fillValues(i, levelSync, samplesBroadSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesBroadSync);
  }
  
  // Half a line of ShortSync
  void halfShortSync(int &i)
  {
    fillValues(i, levelSync, samplesShortSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesShortSync);  
  }
  
  // A full line of Blank
  void fullBlankLine()
  {
    int i = 0;
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesLine - samplesHSync);
  }

  // A full line of Black pixels
  void fullBlackLine()
  {
    int i = 0;
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesBackPorch);
    fillValues(i, levelBlack, samplesActive);
    fillValues(i, levelBlank, samplesFrontPorch);
  }

  // A line of 50% Blank - 50% Black pixels
  void fullHalfBlankHalfBlackLine()
  {
    int i = 0;
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesHSync);
    fillValues(i, levelBlack, samplesLine / 2 - samplesFrontPorch);
    fillValues(i, levelBlank, samplesFrontPorch);
  }

  void halfBlack(int &i)
  {
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesBackPorch);
    fillValues(i, levelBlack, samplesLine / 2 - (samplesHSync + samplesBackPorch));  
  }

  void sendFrameHalfResolution(unsigned char ** pixels) {
    //Even field
    // 4 long
    int i = 0;
    halfBroadSync(i); halfBroadSync(i);
    sendLine(); sendLine();
    // 1 long + 1 short
    i = 0;
    halfBroadSync(i); halfShortSync(i);
    sendLine();
    // 4 short
    i = 0;
    halfShortSync(i); halfShortSync(i);
    sendLine(); sendLine();

    // 17 blank
    fullBlankLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine();

    // Half blank, Half black
    fullHalfBlankHalfBlackLine();
    sendLine();

    // Black lines for vertical centering
    fullBlackLine();
    for(int y = 0; y < linesBlackTop; y++)
      sendLine();

    // Lines (image)
    for(int y = 0; y < height / 2; y++)
    {
      fillLine((* pixels), y * width / 2);
      sendLine();
    }

    // Black lines for vertical centering
    fullBlackLine();
    for(int y = 0; y < linesBlackBottom; y++)
      sendLine();

    // 4 short
    i = 0;
    halfShortSync(i); halfShortSync(i);
    sendLine(); sendLine();

    // 1 short, 1 long 
    i = 0;
    halfShortSync(i);
    // Odd field starts with 1 short
    halfBroadSync(i);
    sendLine();

    // 4 long
    i = 0;
    halfBroadSync(i); halfBroadSync(i);
    sendLine(); sendLine();

    // 4 short
    i = 0;
    halfShortSync(i); halfShortSync(i);
    sendLine(); sendLine();
    
    // 1 short, half a line of blank
    i = 0;
    halfShortSync(i);
    fillValues(i, levelBlank, samplesLine / 2);
    sendLine();

    // 17 blank
    fullBlankLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine(); sendLine();
    sendLine(); sendLine();

    // Black lines for vertical centering
    fullBlackLine();
    for(int y = 0; y < linesBlackTop; y++)
      sendLine();
    // Lines (image)
    for(int y = 0; y < height / 2; y++)
    {
      fillLine((* pixels), y * width / 2);
      sendLine();
    }
    
    // Black lines for vertical centering
    fullBlackLine();
    for(int y = 0; y < linesBlackBottom; y++)
      sendLine();

    // Half black, Half short
    i = 0;
    halfBlack(i); halfShortSync(i);
    sendLine();

    // 4 short
    i = 0;
    halfShortSync(i); halfShortSync(i);
    sendLine(); sendLine();
  }
};
