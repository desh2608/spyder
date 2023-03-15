// spyder/bindings.cc

// Copyright 2021  Johns Hopkins University (Author: Desh Raj)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "containers.h"
#include "der.h"

namespace py = pybind11;

PYBIND11_MODULE(_spyder, m) {
  m.doc() = R"doc(
        Python module
        -----------------------
        .. currentmodule:: _spyder
        .. autosummary::
           :toctree: _generate

           compute_der
    )doc";

  py::class_<spyder::Turn>(m, "Turn").def(py::init<std::string, double, double>());

  py::class_<spyder::TurnList>(m, "TurnList").def(py::init([](py::list turns_) {
    std::vector<spyder::Turn> turns;
    for (auto turn : turns_) turns.push_back(py::cast<spyder::Turn>(turn));

    return new spyder::TurnList(turns);
  }));

  py::class_<spyder::Metrics>(m, "Metrics")
      .def(py::init<double, double, double, double>())
      .def_readwrite("duration", &spyder::Metrics::duration,
                     py::return_value_policy::copy)
      .def_readwrite("miss", &spyder::Metrics::miss, py::return_value_policy::copy)
      .def_readwrite("falarm", &spyder::Metrics::falarm, py::return_value_policy::copy)
      .def_readwrite("conf", &spyder::Metrics::conf, py::return_value_policy::copy)
      .def_readwrite("der", &spyder::Metrics::der, py::return_value_policy::copy);

  m.def("compute_der", &spyder::compute_der, py::return_value_policy::reference,
        py::arg("ref"), py::arg("hyp"), py::arg("uem"), py::pos_only(),
        py::arg("regions") = "all", py::arg("collar") = 0.0,
        R"doc(Compute DER metrics)doc");
}
