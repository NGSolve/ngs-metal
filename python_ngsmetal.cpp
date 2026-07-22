#include <comp.hpp>
#include <python_comp.hpp>

#include "ngsmetal.hpp"

PYBIND11_MODULE(ngsmetal, m)
{
  cout << "Loading ngs-metal library" << endl;

  ngsmetal::InitNgsMetal();

}
