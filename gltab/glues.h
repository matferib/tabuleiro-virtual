// Funcoes do glues.

#include "gltab/gl.h"

#define __glPi 3.14159265358979323846

namespace gl {
namespace glu {

void PreencheIdentidade(GLfloat m[16]);
void Normaliza(GLfloat v[3]);
void ProdutoVetorial(GLfloat v1[3], GLfloat v2[3], GLfloat result[3]);
int InvertMatrixf(const GLfloat m[16], GLfloat invOut[16]);
void MultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4], GLfloat out[4]);
void MultMatricesf(const GLfloat a[16], const GLfloat b[16], GLfloat r[16]);
GLint Project(
    GLfloat objx,
    GLfloat objy,
    GLfloat objz,
    const GLfloat modelMatrix[16],
    const GLfloat projMatrix[16],
    const GLint viewport[4],
    GLfloat* winx,
    GLfloat* winy,
    GLfloat* winz);
GLint Unproject(GLfloat winx, GLfloat winy, GLfloat winz,
                const GLfloat modelMatrix[16],
                const GLfloat projMatrix[16],
                const GLint viewport[4],
                GLfloat* objx, GLfloat* objy, GLfloat* objz);

}  // namespace glu
}  // namespace gl
