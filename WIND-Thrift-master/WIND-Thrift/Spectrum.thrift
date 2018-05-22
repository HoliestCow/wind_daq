namespace cpp wind
namespace java org.wind
namespace csharp org.wind

struct ListMode
{
	1: i64 eventTime //Nanoseconds since timeStamp
	2: i32 channel
}

/**
 * Spectrum formats.  Sensors may serialize data using one
 * of the following data formats.
 **/
enum SpectrumFormat {
  /**
   * Channel data is an array of tuples.  Tuples are arranged in
   * the array such that the first value is the channel number
   * and the value is the channel count.  Channels with
   * no counts should be omitted.
   **/
  CHANNEL_VALUE = 0x0001;
  /**
   * Channel data is an array where each value
   * represents the next count and the index represents the channel
   * number.  Channels with no counts must not be omitted.
   **/
  ARRAY = 0x0002;
  /**
   * Channel data is an array where each value
   * represents the next count and the index represents the channel
   * number.  Channels with no counts are represented by a negative
   * number the value of which represents the number of no count
   * channels in the string between channels with counts.  If channel
   * 10 has 4 counts, channel 11 has no counts, and channel 12 has
   * 5 counts, the values for [10 - 12] should be [4, -1, 5].  If
   * channel 10 has 4 counts, channels 11 through 15 have no counts,
   * and channel 16 has 5 counts, the values for [10 - 16] should be
   * [4, -5, 5]
   */
  ZERO_PACK = 0x0004;
}

struct DoubleSpectrum
{
  /** The spectrum data itself **/
  1: list<double> spectrumDouble;

   /** 
    * The format of the data in channelData 
    **/
  2: SpectrumFormat format; 

   /**
    *The resolution of the spectra
    **/ 
  3: i32 channelCount;
  /** Live time in milliseconds */
  4: i64 liveTime;
}

struct Spectrum
{
  /** The spectrum data itself. Implementations should fill only one
   * of these values
   **/
  1: list<i32> spectrumInt;

   /** 
    * The format of the data in channelData 
    **/
  2: SpectrumFormat format;   

   /**
    *The resolution of the spectra
    **/ 
  3: i32 channelCount;
  /** Live time in milliseconds */
  4: i64 liveTime;
}

/** Used when result can be either integer spectrum or double spectrum */
union SpectrumResult
{
  1: Spectrum intSpectrum;
  2: DoubleSpectrum doubleSpectrum;
}
