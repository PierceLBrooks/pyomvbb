
#include "converter.hpp"
#include "ApproxMVBB/ComputeApproxMVBB.hpp"
#include <boost/python.hpp>
#include <vector>
#include <tuple>
#include <map>

namespace py = boost::python;

typedef std::tuple<float, float, float> Point;
typedef std::vector<Point> PointVector;

struct OMVBB
{
    OMVBB(float extension)
    {
        this->extension = static_cast<double>(extension);
    }
    void compute(PointVector points)
    {
        ApproxMVBB::Matrix3Dyn matrix(3, points.size());
        for (unsigned int i = 0; i < points.size(); ++i)
        {
            matrix(0, i) = static_cast<double>(std::get<0>(points[i]));
            matrix(1, i) = static_cast<double>(std::get<1>(points[i]));
            matrix(2, i) = static_cast<double>(std::get<2>(points[i]));
        }
        ApproxMVBB::OOBB box = ApproxMVBB::approximateMVBB(matrix,
                                                        0.001,
                                                        500,
                                                        5,
                                                        0,
                                                        5);
        // To make all points inside the OOBB :
        ApproxMVBB::Matrix33 transpose = box.m_q_KI.matrix().transpose();  // faster to store the transformation matrix first
        auto size = matrix.cols();
        for(unsigned int i = 0; i < size; ++i)
        {
            box.unite(transpose * matrix.col(i));
        }
        if (extension > 0.0)
        {
            box.expandToMinExtentAbsolute(extension);
        }
    }
    double extension;
};

BOOST_PYTHON_MODULE(omvbb)
{
    Py_Initialize();
    
    py::class_<OMVBB>("OMVBB", py::init<float>())
        .def("compute", &OMVBB::compute)
    ;
    
    tuple_tuple_converter<float, float, float>::register_bidirectional_converter();
    list_vector_converter<Point>::register_bidirectional_converter();
}

