#ifndef __perlin_h__
#define __perlin_h__

class Perlin
{
  public:
    Perlin (void);
    void init (void);
    void next (uint8_t *levels);
    ~Perlin (void);

  private:
    int32_t m_width;
    int32_t m_height;
    int32_t m_mode;
    int32_t m_xy_scale;
    float   m_z_step;
    float   m_z_depth;
    float   m_hue_options;
    float   m_z_state;
    float   m_hue_state;
    float   m_min, m_max;

    int32_t noise (uint16_t x, uint16_t y, uint16_t z);
    void translate_hue (int16_t hue, uint8_t *rgb);
};

#endif
