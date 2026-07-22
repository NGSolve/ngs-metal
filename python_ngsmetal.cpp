#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>


#include <comp.hpp>
#include <python_comp.hpp>

#include "ngsmetal.hpp"
#include "metal_vector.hpp"

using namespace ngsmetal;


PYBIND11_MODULE(ngsmetal, m)
{
  cout << "Loading ngs-metal library" << endl;

  ngsmetal::InitNgsMetal();
  ngsmetal::InitVectorKernels();

  py::class_<MetalVector, BaseVector, shared_ptr<MetalVector>> (m, "MetalVector")
    .def(py::init<size_t>(), py::arg("size"))
    ;
}
