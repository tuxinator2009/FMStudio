#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

struct Chunk
{
  enum class Type {Null, MThd, MTrk};
  Chunk()
  {
    for (int i = 0; i < 5; ++i)
      typeChar[i] = '\0';
    size = 0;
    data = nullptr;
  };
  ~Chunk()
  {
    if (data != nullptr)
      delete[] data;
  }
  Type type;
  char typeChar[5];
  uint32_t size;
  uint8_t *data;
};

inline uint32_t read32(uint8_t *data)
{
  return ((uint32_t)data[0] << 24) + ((uint32_t)data[1] << 16) + ((uint32_t)data[2] << 8) + (uint32_t)data[3];
}

inline uint32_t read24(uint8_t *data)
{
  return ((uint32_t)data[0] << 16) + ((uint32_t)data[1] << 8) + (uint32_t)data[2];
}

inline uint16_t read16(uint8_t *data)
{
  return ((uint32_t)data[0] << 8) + (uint32_t)data[1];
}

uint32_t readVariableLength(uint8_t *data, uint32_t *i)
{
  uint32_t value = data[*i];
  if (value & 0x80)
  {
    value &= 0x7f;
    do
    {
      ++(*i);
      value = (value << 7) + (data[*i] & 0x7f);
    } while (data[*i] & 0x80);
  }
  ++(*i);
  return value;
}

Chunk nextChunk(FILE *file)
{
  Chunk chunk;
  uint8_t sizeBytes[4];
  fread(chunk.typeChar, 1, 4, file);
  if (strcmp(chunk.typeChar, "MThd") == 0)
    chunk.type = Chunk::Type::MThd;
  else if (strcmp(chunk.typeChar, "MTrk") == 0)
    chunk.type = Chunk::Type::MTrk;
  else
    chunk.type = Chunk::Type::Null;
  fread(sizeBytes, 1, 4, file);
  chunk.size = read32(sizeBytes);
  chunk.data = new uint8_t[chunk.size];
  fread(chunk.data, 1, chunk.size, file);
  return chunk;
}

void processMetaEvent(uint8_t *data, uint32_t *i)
{
  uint8_t type = data[*i];
  uint8_t length = data[*i + 1];
  uint8_t *bytes = data + *i + 2;
  *i += length + 2;
  if (type == 0x00) //Sequence number
  {
    if (length == 0x02)
      printf("Sequence number %d\n", read16(bytes));
    else if (length == 0x00)
      printf("Sequence number ordered\n");
  }
  else if (type == 0x01) //Text
    printf("Text %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x02) //Copyright
    printf("Copyright %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x03) //Track name
    printf("Track name %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x04) //Instrument name
    printf("Instrument name %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x05) //Lyric
    printf("Lyric %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x06) //Marker
    printf("Marker %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x07) //Cue Point
    printf("Cue Point %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x08) //Program Name
    printf("Program name %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x09) //Device (port) Name
    printf("Device (port) name %d \"%*s\"\n", length, length, bytes);
  else if (type == 0x20) //Channel prefix
    printf("Channel prefix %d\n", bytes[0]);
  else if (type == 0x21) //MIDI port
    printf("MIDI port %d\n", bytes[0]);
  else if (type == 0x2f) //End of Track
    printf("End of Track\n");
  else if (type == 0x4b) //M-Live Tag
  {
    uint8_t tag = bytes[0];
    if (tag == 0x01)
      printf("M-Live Genre ");
    else if (tag == 0x02)
      printf("M-Live Artist ");
    else if (tag == 0x03)
      printf("M-Live Composer ");
    else if (tag == 0x04)
      printf("M-Live Duration (seconds) ");
    else if (tag == 0x05)
      printf("M-Live BPM ");
    printf("\"%*s\"\n", length - 1, bytes + 1);
  }
  else if (type == 0x51) //Tempo
    printf("Tempo %u usecs / quarter note (%d BPM)\n", read24(bytes), 60000000 / read24(bytes));
  else if (type == 0x54) //SMPTE offset
  {
    uint8_t frameRate = bytes[0] >> 5;
    uint8_t hour = bytes[0] & 0x1f;
    uint8_t min = bytes[1];
    uint8_t sec = bytes[2];
    uint8_t frm = bytes[3];
    uint8_t sfrm = bytes[4];
    printf("SMPTE offset (");
    if (frameRate == 0)
      printf("24fps ");
    else if (frameRate == 1)
      printf("25fps ");
    else if (frameRate == 2)
      printf("30fps (drop frame) ");
    else if (frameRate == 3)
      printf("30fps ");
    printf("%02d:%02d:%02d %02d.%02d)\n", hour, min, sec, frm, sfrm);
  }
  else if (type == 0x58) //Time signature
    printf("Time signature (%d/%d, %d, %d)\n", bytes[0], 1 << bytes[1], bytes[2], bytes[3]);
  else if (type == 0x59) //Key signature
  {
    int8_t key = ((int8_t*)bytes)[0];
    uint8_t minor = bytes[1];
    printf("Key signature (%d %s)\n", key, (minor == 1) ? "minor":"major");
  }
  else if (type == 0x7f) //Sequencer Specific
  {
    printf("Sequencer Specific Data %d\n   ", length);
    for (int j = 0; j < length; ++j)
    {
      printf(" %02X", bytes[j]);
      if (j % 32 == 31)
        printf("\n   ");
    }
    if (length % 32 != 0)
      printf("\n");
  }
  else
  {
    printf("Unknown: %02X with %d bytes\n   ", type, length);
    for (int j = 0; j < length; ++j)
    {
      printf(" %02X", bytes[j]);
      if (j % 32 == 31)
        printf("\n   ");
    }
    if (length % 32 != 0)
      printf("\n");
  }
}

void processSystemMessage(uint8_t *data, uint32_t *i, uint8_t type)
{
  if (type == 0) //0000: System Exclusive
  {
    int j = 0;
    uint8_t id = data[*i];
    printf("System exclusive id=%d\n   ", id);
    do
    {
      *i += 1;
      printf(" %02X", data[*i]);
      ++j;
      if (j % 32 == 31)
        printf("\n   ");
    } while (data[*i] != 0xF7);
    if (j % 32 != 0)
      printf("\n");
    *i += 1;
  }
  else if (type == 1) //0001: Undefined
    printf("Undefined system message %u\n", type);
  else if (type == 2) //0010: Song position pointer
  {
    uint16_t value = ((data[*i] & 0x7f) << 7) + (data[*i + 1] & 0x7f);
    i += 2;
    printf("Song position pointer %u\n", value);
  }
  else if (type == 3) //0011: Song Select
    printf("Song select %d\n", data[*i++]);
  else if (type == 4) //0100: Undefined
    printf("Undefined system message %u\n", type);
  else if (type == 5) //0101: Undefined
    printf("Undefined system message %u\n", type);
  else if (type == 6) //0110: Tune Request
    printf("Tune request\n");
  else if (type == 7) //0111: End of Exclusive
    printf("End of exclusive\n");
  else if (type == 8) //1000: Timing Clock
    printf("Timing Clock\n");
  else if (type == 9) //1001: Undefined
    printf("Undefined system message %u\n", type);
  else if (type == 10) //1010: Start
    printf("Start\n");
  else if (type == 11) //1011: Continue
    printf("Continue\n");
  else if (type == 12) //1100: Stop
    printf("Stop\n");
  else if (type == 13) //1101: Undefined
    printf("Undefined system message %u\n", type);
  else if (type == 14) //1110: Active Sensing
    printf("Active sensing\n");
  else if (type == 15) //1111: Reset
    printf("Reset\n");
}

void processChannelEvent(uint8_t *data, uint32_t *i, uint8_t event)
{
  uint8_t type = (event & 0xF0);
  uint8_t channel = (event & 0x0F);
  if (type != 0xF0)
    printf("ch%d ", channel);
  if (type == 0x80) //Note-off
  {
    uint8_t note = data[*i];
    uint8_t velocity = data[*i + 1];
    *i += 2;
    printf("Note-off note=%d velocity=%d\n", note, velocity);
  }
  else if (type == 0x90) //Note-on
  {
    uint8_t note = data[*i];
    uint8_t velocity = data[*i + 1];
    *i += 2;
    printf("Note-on note=%d velocity=%d\n", note, velocity);
  }
  else if (type == 0xa0) //Aftertouch Event
  {
    uint8_t note = data[*i];
    uint8_t amount = data[*i + 1];
    *i += 2;
    printf("Note aftertouch note=%d amount=%d\n", note, amount);
  }
  else if (type == 0xb0) //Controller Event
  {
    uint8_t controller = data[*i];
    uint8_t value = data[*i + 1];
    *i += 2;
    printf("Controller 0x%02X=%d\n", controller, value);
  }
  else if (type == 0xc0) //Program Change Event
  {
    uint8_t program = data[*i];
    *i += 1;
    printf("Program change 0x%02X (%d)\n", program, program);
  }
  else if (type == 0xd0) //Channel Aftertouch Event
  {
    uint8_t amount = data[*i];
    *i += 1;
    printf("Channel aftertouch amount=%d\n", amount);
  }
  else if (type == 0xe0) //Pitch Bend Event
  {
    uint16_t value = ((data[*i] & 0x7f) << 7) + (data[*i + 1] & 0x7f);
    i += 2;
    printf("Pitch bend %u\n", value);
  }
  else if (type == 0xf0) //System Messages
    processSystemMessage(data, i, channel);
  else
    printf("Unknown %02X\n", type);
}

int main(int argc, char *argv[])
{
  FILE *file;
  long fileEnd;
  if (argc != 2)
  {
    printf("usage: %s file.mid\n", argv[0]);
    return -1;
  }
  file = fopen(argv[1], "rb");
  if (!file)
  {
    printf("Error: failed to open midi file\n");
    perror("Reason:");
    return -1;
  }
  fseek(file, 0, SEEK_END);
  fileEnd = ftell(file);
  fseek(file, 0, SEEK_SET);
  while (ftell(file) != fileEnd)
  {
    Chunk chunk = nextChunk(file);
    printf("Chunk:\n");
    printf("  Type: %s\n", chunk.typeChar);
    printf("  Size: %u bytes\n", chunk.size);
    if (chunk.type == Chunk::Type::MThd && chunk.size >= 6)
    {
      printf("  Format: %d\n", read16(chunk.data));
      printf("  Tracks: %d\n", read16(chunk.data + 2));
      printf("  Tempo: %d ticks per quarter note\n", read16(chunk.data + 4));
    }
    else if (chunk.type == Chunk::Type::MTrk)
    {
      uint32_t i = 0;
      uint8_t event;
      while (i < chunk.size)
      {
        uint32_t start = i;
        uint32_t deltaTime = readVariableLength(chunk.data, &i);
        printf(" ");
        for (uint32_t j = 4; j > i - start; --j)
          printf(" ..");
        for (uint32_t j = start; j < i; ++j)
          printf(" %02X", chunk.data[j]);
        if (chunk.data[i] & 0x80)
          event = chunk.data[i++];
        printf(" (%u): %02X ", deltaTime, event);
        if (event == 0xFF) //Meta events
          processMetaEvent(chunk.data, &i);
        else
          processChannelEvent(chunk.data, &i, event);
      }
    }
  }
  fclose(file);
  return 0;
}
