#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

// Include your headers (adjust paths as needed)
#include "../src/core/TUI-Rendering/Geometry/object/Matrix.cpp"
#include "../src/core/TUI-Rendering/Geometry/object/Vector.cpp"

using namespace Geometry;

void printVector(const Vector& v, const std::string& name) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << name << " = (" << v.getX() << ", " << v.getY() << ", " << v.getZ() << ", " << v.getW() << ")" << std::endl;
}

void printMatrix(const Matrix& m, const std::string& name) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << name << ":" << std::endl;
    auto values = m.getValue();
    for (int row = 0; row < 4; ++row) {
        std::cout << "  ";
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(8) << values[row * 4 + col] << " ";
        }
        std::cout << std::endl;
    }
}

void testVectorConstructors() {
    std::cout << "=== Testing Vector Constructors ===" << std::endl;
    
    Vector v1;  // Default constructor
    printVector(v1, "Default Vector");
    
    Vector v2(1.0f, 2.0f, 3.0f, true);  // Vector (w=0)
    printVector(v2, "Vector (w=0)");
    
    Vector v3(4.0f, 5.0f, 6.0f, false);  // Point (w=1)
    printVector(v3, "Point (w=1)");
    
    Vector v4(7.0f, 8.0f, 9.0f, 0.5f);  // Homogeneous coordinates
    printVector(v4, "Homogeneous Vector");
    
    Vector v5(v3);  // Copy constructor
    printVector(v5, "Copy of Point");
    
    std::cout << std::endl;
}

void testVectorOperations() {
    std::cout << "=== Testing Vector Arithmetic Operations ===" << std::endl;
    
    Vector point1(1.0f, 2.0f, 3.0f, false);  // w=1
    Vector point2(4.0f, 5.0f, 6.0f, false);  // w=1
    Vector vec1(1.0f, 0.0f, 0.0f, true);     // w=0
    Vector vec2(0.0f, 1.0f, 0.0f, true);     // w=0
    
    // Test addition
    try {
        Vector vecSum = vec1 + vec2;
        printVector(vecSum, "vec1 + vec2");
        
        // This should throw an exception
        Vector pointSum = point1 + point2;
        printVector(pointSum, "point1 + point2");
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    // Test subtraction
    try {
        Vector pointDiff = point1 - point2;  // Should throw
        printVector(pointDiff, "point1 - point2");
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    Vector vecDiff = vec1 - vec2;
    printVector(vecDiff, "vec1 - vec2");
    
    // Test scalar multiplication
    Vector scaled = vec1 * 2.5f;
    printVector(scaled, "vec1 * 2.5");
    
    // Test scalar division
    Vector divided = vec1 / 2.0f;
    printVector(divided, "vec1 / 2.0");
    
    // Test compound operations
    Vector v(1.0f, 2.0f, 3.0f, true);
    v += Vector(0.5f, 0.5f, 0.5f, true);
    printVector(v, "v += (0.5, 0.5, 0.5)");
    
    v *= 2.0f;
    printVector(v, "v *= 2.0");
    
    std::cout << std::endl;
}

void testVectorProperties() {
    std::cout << "=== Testing Vector Properties and Operations ===" << std::endl;
    
    Vector v1(3.0f, 4.0f, 0.0f, true);
    Vector v2(1.0f, 2.0f, 3.0f, true);
    Vector v3(0.0f, 0.0f, 1.0f, true);
    
    // Length and norm
    std::cout << "v1 length: " << v1.length() << std::endl;
    std::cout << "v1 norm: " << v1.Norm() << std::endl;
    std::cout << "v1 length squared: " << v1.lengthSq() << std::endl;
    
    // Dot product
    float dot = v1.Dot(v2);
    std::cout << "v1 · v2 = " << dot << std::endl;
    
    // Cross product
    Vector cross = v2.Cross(v3);
    printVector(cross, "v2 × v3");
    
    // Test getRealPosition
    Vector point(2.0f, 4.0f, 6.0f, 2.0f);  // Homogeneous coordinates
    try {
        Vector realPos = point.getRealPosition();
        printVector(realPos, "Real position of point");
        
        // This should throw
        Vector vec(1.0f, 2.0f, 3.0f, true);
        vec.getRealPosition();
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

void testMatrixConstructors() {
    std::cout << "=== Testing Matrix Constructors ===" << std::endl;
    
    Matrix m1;  // Identity matrix
    printMatrix(m1, "Identity Matrix");
    
    // Custom matrix
    std::array<float, 16> custom = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
    Matrix m2(custom);
    printMatrix(m2, "Custom Matrix");
    
    std::cout << std::endl;
}

void testMatrixOperations() {
    std::cout << "=== Testing Matrix Operations ===" << std::endl;
    
    std::array<float, 16> arr1 = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
    
    std::array<float, 16> arr2 = {
        16.0f, 15.0f, 14.0f, 13.0f,
        12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f,
        4.0f, 3.0f, 2.0f, 1.0f
    };
    
    Matrix m1(arr1);
    Matrix m2(arr2);
    
    // Addition
    Matrix sum = m1 + m2;
    printMatrix(sum, "m1 + m2");
    
    // Subtraction
    Matrix diff = m1 - m2;
    printMatrix(diff, "m1 - m2");
    
    // Scalar multiplication
    Matrix scaled = m1 * 2.0f;
    printMatrix(scaled, "m1 * 2.0");
    
    // Scalar division
    Matrix divided = m1 / 2.0f;
    printMatrix(divided, "m1 / 2.0");
    
    // Matrix multiplication
    Matrix prod = m1 * m2;
    printMatrix(prod, "m1 * m2");
    
    // Compound operations
    Matrix m = Matrix::Identity();
    m += m1;
    printMatrix(m, "Identity + m1");
    
    m *= 0.5f;
    printMatrix(m, "Result * 0.5");
    
    // Transpose
    Matrix transposed = m1.transpose();
    printMatrix(transposed, "Transpose of m1");
    
    // Equality
    std::cout << "m1 == m2: " << (m1 == m2) << std::endl;
    std::cout << "m1 != m2: " << (m1 != m2) << std::endl;
    
    // Index operator
    std::cout << "m1[5] = " << m1[5] << std::endl;
    m1[5] = 100.0f;
    std::cout << "After m1[5] = 100.0: " << m1[5] << std::endl;
    
    std::cout << std::endl;
}

void testTransformations() {
    std::cout << "=== Testing Transformations ===" << std::endl;
    
    // Create transformation matrices
    Matrix translation = Matrix::createTranslation(1.0f, 2.0f, 3.0f);
    printMatrix(translation, "Translation Matrix");
    
    Matrix scale = Matrix::createScale(2.0f, 3.0f, 4.0f);
    printMatrix(scale, "Scale Matrix");
    
    float angle = M_PI / 4.0f;  // 45 degrees
    Matrix rotationX = Matrix::createRotationX(angle);
    printMatrix(rotationX, "Rotation X Matrix");
    
    Matrix rotationY = Matrix::createRotationY(angle);
    printMatrix(rotationY, "Rotation Y Matrix");
    
    Matrix rotationZ = Matrix::createRotationZ(angle);
    printMatrix(rotationZ, "Rotation Z Matrix");
    
    // Test vector transformations
    Vector point(1.0f, 2.0f, 3.0f, false);
    Vector vec(1.0f, 0.0f, 0.0f, true);
    
    // Translate point
    Vector translatedPoint = point.Transform(translation);
    printVector(translatedPoint, "Translated point");
    
    // Scale vector
    Vector scaledVec = vec.Transform(scale);
    printVector(scaledVec, "Scaled vector");
    
    // Rotate vector
    Vector rotatedVec = vec.Rotate(Vector(angle, 0.0f, 0.0f, true));
    printVector(rotatedVec, "Rotated vector around X");
    
    // Combined transformation
    Matrix combined = translation * rotationZ * scale;
    Vector transformed = point.Transform(combined);
    printVector(transformed, "Combined transformation");
    
    Vector result = point * translation;
    printVector(result, "point * translation");
    
    std::cout << std::endl;
}

void testEdgeCases() {
    std::cout << "=== Testing Edge Cases ===" << std::endl;
    
    // Test division by zero
    Vector v(1.0f, 2.0f, 3.0f, true);
    try {
        Vector divByZero = v / 0.0f;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught division by zero: " << e.what() << std::endl;
    }
    
    // Test matrix division by zero
    Matrix m = Matrix::Identity();
    try {
        Matrix badDiv = m / 0.0f;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught matrix division by zero: " << e.what() << std::endl;
    }
    
    // Test out of bounds index access
    try {
        float val = m[16];  // Out of bounds
    } catch (const std::out_of_range& e) {
        std::cout << "Caught out of bounds access: " << e.what() << std::endl;
    }
    
    // Test assignment operator
    Vector v1(1.0f, 2.0f, 3.0f, true);
    Vector v2(4.0f, 5.0f, 6.0f, false);
    v1 = v2;
    printVector(v1, "After assignment v1 = v2");
    
    // Self-assignment
    v1 = v1;
    printVector(v1, "After self-assignment");
    
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Testing Geometry Library ===" << std::endl << std::endl;
    
    testVectorConstructors();
    testVectorOperations();
    testVectorProperties();
    
    testMatrixConstructors();
    testMatrixOperations();
    
    testTransformations();
    testEdgeCases();
    
    std::cout << "=== All Tests Complete ===" << std::endl;
    
    return 0;
}