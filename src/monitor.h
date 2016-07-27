template <typename T>
inline void memset_T(T* b, T c, unsigned size) {
  while(size-->0)
    *(b++) = c;
}
#define _Slots 7
#define _TOTAL 93
template <typename _ValueType>
class Monitor {
public:
  typedef _ValueType ValueType;
  Monitor() {
    Sizes[0] = 15;//15s
    Sizes[1] = 16;//4m
    Sizes[2] = 15;//1h
    Sizes[3] = 24;//1d
    Sizes[4] = 7;//7d (1w)
    Sizes[5] = 8;//56d (~2mo)
    Sizes[6] = 8;//448d (>1y)
    Offsets[0] = 0;
    for (unsigned char i = 0; i < _Slots-1; ++i) {
      Offsets[i + 1] = Offsets[i] + Sizes[i];
    }
    memset_T(&LastSeconds[0], (ValueType)0, _TOTAL);
    memset_T(&LastUpdate[0], (unsigned long)0, _Slots);
  }
  void PutData(unsigned long _time, unsigned long _value) {
    _PutData(_time, _value, 0);
  }

  ValueType GetData(unsigned long _timeOffset, unsigned long _period) {
    return _GetData(_timeOffset, _period, 0);
  }

  ValueType NextAlignedData(unsigned long _timeOffset, unsigned long _period, unsigned long& _realPeriod, unsigned long& lu) {
    lu = GetLastUpdated() - _timeOffset;
    _realPeriod = lu % (_period);
    if (_realPeriod == 0)
      _realPeriod = _period;
    return GetData(_timeOffset, _realPeriod);
  }

  unsigned long GetLastUpdated() const {
    return LastUpdate[0];
  }
  
private:
  ValueType Avergade(unsigned char _slot, unsigned char _offset, unsigned char _size) {
    unsigned long accumulated = 0;
    const unsigned lineOffset = Offsets[_slot];
    for (unsigned char i = 0; i < _size; i++, ++_offset) {
      accumulated += LastSeconds[lineOffset + _offset];
    }
    return (ValueType)(accumulated/_size);
  }

  void Extrude(unsigned long _time, unsigned char _slot) {
    _PutData(_time / Sizes[_slot], Avergade(_slot, 0, Sizes[_slot]), _slot + 1);
  }

  void _PutData(unsigned long _time, unsigned long _value, unsigned char _slot) {
    if (_slot >= _Slots) {
      return;
    }
    unsigned char slotSize = Sizes[_slot];
    unsigned char slotOffset = Offsets[_slot];
    unsigned long lastUpdate = LastUpdate[_slot];
    unsigned long diff = _time - lastUpdate;
    unsigned char newPos = _time % slotSize;
    unsigned char lastPos = lastUpdate % slotSize;
    unsigned char firstFreePos = (lastUpdate + 1) % slotSize;
    if (lastPos > newPos || diff > slotSize) {
      if (firstFreePos) {
        ValueType fillValue = (_value*(diff%slotSize) + LastSeconds[slotOffset + lastPos] * (diff)) / (diff + diff%slotSize);
        memset_T(&LastSeconds[slotOffset + firstFreePos], fillValue, slotSize - firstFreePos);
        firstFreePos = 0;
      }
      Extrude(lastUpdate, _slot);
    }
    if (diff > 1) {
      ValueType fillValue = (_value*(diff%slotSize) + LastSeconds[slotOffset + lastPos] * (diff)) / (diff + diff%slotSize);
      memset_T(&LastSeconds[slotOffset + firstFreePos], fillValue, newPos - firstFreePos);
    }
    
    LastSeconds[slotOffset + newPos] = _value;
    LastUpdate[_slot] = _time;
  }

  ValueType _GetData(unsigned long _timeOffset, unsigned long _period, unsigned char _slot) {
    if (_period == 0)
      _period = 1;
    ValueType accumulated = 0;
    unsigned char slotSize = Sizes[_slot];
    unsigned long restPeriod = _period;
    if (_timeOffset <= slotSize) {
      unsigned long lastUpdate = LastUpdate[_slot];
      unsigned char pos = (lastUpdate - _timeOffset) % slotSize;
      unsigned char lastPos = lastUpdate % slotSize;
      unsigned pp = pos < restPeriod ? pos : restPeriod;
      if (pp) {
        accumulated = Avergade(_slot, (unsigned char)(pos - pp + 1), pp);
        restPeriod -= pp;
        if (!restPeriod)
          return accumulated;
      }
      unsigned char ss = slotSize - lastPos;
      if (ss >= restPeriod)
        return Avergade(_slot, slotSize - ss, (unsigned char)restPeriod);
    }
    ValueType awayAccumulated = _GetData(_timeOffset / slotSize, restPeriod / slotSize, _slot + 1);
    if (_period > 256 || _timeOffset >= slotSize)
      return awayAccumulated;
    return (ValueType)(accumulated*(_period - restPeriod) + (restPeriod)* awayAccumulated) / _period;
  }
  unsigned long LastUpdate[_Slots];
  ValueType LastSeconds[_TOTAL];
  unsigned char Sizes[_Slots];
  unsigned char Offsets[_Slots];
};
