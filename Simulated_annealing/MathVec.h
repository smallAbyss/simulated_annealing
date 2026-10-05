#include <vector>
#include <iostream>
#include <initializer_list>
#include <stdexcept>

template <typename T>
class MathVec {
    std::vector<T> data;

public:
    // default constructor: vector of 0 size()
    MathVec() = default;

    MathVec(const MathVec& vec) {
        this->operator=(vec);
    }

    MathVec& operator=(const MathVec& vec) {
        if (this != &vec) {
            data = vec.data;  // there're vectors, so it's ok 
        }
        return *this;
    }

    // 0-vec via dim
    MathVec(size_t dim) {
        data.resize(dim);
        for (size_t i = 0; i < dim; i++)
            (*this)[i] = 0;
    }  

    // via Initializer list
    MathVec(std::initializer_list<T>& init) : data(init) {}

    // via Initializer std::vector
    explicit MathVec(const std::vector<T>& vec) : data(vec) {}

    const std::vector<T>& GetRawVector() const {
        return data;
    }

    size_t size() const {
        return data.size();
    }

    void print() const {
        std::cout << "MathVec: [ ";
        for (double val : data) {
            std::cout << val << " ";
        }
        std::cout << "]\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const MathVec& vec) {
        os << "[";
        for (size_t i = 0; i < vec.data.size(); ++i) {
            os << vec.data[i];
            if (i < vec.data.size() - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

    T& operator[](size_t index) {
        if (index >= size()) 
            throw std::invalid_argument("Index out of range");
        return data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size()) 
            throw std::invalid_argument("Index out of range");
        return data[index];
    }

    // --- vec arithmetic --- //

    MathVec operator+(const MathVec<T>& b) const {
        if (size() != b.size())
            throw std::invalid_argument("Vector sizes must match");
        MathVec result(size());

        for (size_t i =0; i < size(); i++){
            result[i] = (*this)[i] + b[i];
        }
        return result;
    }

    MathVec operator-(const MathVec& b) const {
        if (size() != b.size())
            throw std::invalid_argument("Vector sizes must match");
        MathVec result(size());

        for (size_t i =0; i < size(); i++){
            result[i] = (*this)[i] - b[i];
        }
        return result;
    }

    MathVec operator*(const double k) const {
        MathVec result(size());

        for (size_t i =0; i < size(); i++){
            result[i] = (*this)[i] * k;
        }
        return result;
    }

    friend MathVec operator*(const double k, const MathVec& a) {
        return a*k;
    }
        

};

// int main() {
//     MathVec<double> v1 = {1.0, 2.0, 3.0};
//     MathVec<double> v2 = {4.0, 5.0, 6.0};

//     MathVec<double> v3 = v1 + v2;
//     std::cout << "v1 + v2 = " << v3 << "\n";

//     MathVec<double> v4 = 2.0 * v1;
//     std::cout << "2.0 * v1 = " << v4 << "\n";

//     return 0;
// }