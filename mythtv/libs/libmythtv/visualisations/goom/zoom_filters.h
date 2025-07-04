#include <array>
#include <vector>

using sintvec = std::vector<signed int>;
using GoomCoefficients = std::array<std::array<int,16>,16>;

void    zoom_filter_xmmx (int prevX, int prevY,
                          unsigned int *expix1, unsigned int *expix2,
                          const sintvec& brutS, const sintvec& brutD, int buffratio,
                          GoomCoefficients& precalCoef);
int 	zoom_filter_xmmx_supported (void);
void    zoom_filter_mmx (int prevX, int prevY,
                         const unsigned int *expix1, unsigned int *expix2,
                         const sintvec& brutS, const sintvec& brutD,
                         int buffratio, const GoomCoefficients &precalCoef);
int 	zoom_filter_mmx_supported (void);

