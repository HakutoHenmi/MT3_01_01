#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "LE2B_20_ヘンミ_ハクト";

struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

Matrix4x4 MakeIdentityMatrix()
{
    Matrix4x4 result {};
    for (int i = 0; i < 4; ++i)
        result.m[i][i] = 1.0f;
    return result;
}

Matrix4x4 MakeRotateMatrix(float angle)
{
    Matrix4x4 result = MakeIdentityMatrix();
    result.m[0][0] = cosf(angle);
    result.m[0][2] = sinf(angle);
    result.m[2][0] = -sinf(angle);
    result.m[2][2] = cosf(angle);
    return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t)
{
    Matrix4x4 result = MakeIdentityMatrix();
    result.m[3][0] = t.x;
    result.m[3][1] = t.y;
    result.m[3][2] = t.z;
    return result;
}

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b)
{
    Matrix4x4 result {};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                result.m[i][j] += a.m[i][k] * b.m[k][j];
    return result;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m)
{
    Vector3 result;
    float w;
    result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
    result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
    result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
    w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
    result.x /= w;
    result.y /= w;
    result.z /= w;
    return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ)
{
    Matrix4x4 result = {};
    float f = 1.0f / tanf(fovY / 2.0f);
    result.m[0][0] = f / aspect;
    result.m[1][1] = f;
    result.m[2][2] = farZ / (nearZ - farZ);
    result.m[2][3] = -1.0f;
    result.m[3][2] = (nearZ * farZ) / (nearZ - farZ);
    return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
    Matrix4x4 result = {};
    result.m[0][0] = width / 2.0f;
    result.m[1][1] = -height / 2.0f;
    result.m[2][2] = maxDepth - minDepth;
    result.m[3][0] = left + width / 2.0f;
    result.m[3][1] = top + height / 2.0f;
    result.m[3][2] = minDepth;
    result.m[3][3] = 1.0f;
    return result;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256] = {};
    char preKeys[256] = {};

    Vector3 v1 = { 1.0f, -3.0f, 2.5f };
    Vector3 v2 = { 0.4f, -1.1f, -1.2f };
    Vector3 cross = Cross(v1, v2);

    Vector3 triangle[3] = {
        { 0.0f, -0.5f, 0.0f },
        { 0.5f, 0.5f, 0.0f },
        { -0.5f, 0.5f, 0.0f }
    };

    Vector3 rotate = { 0.0f, 0.0f, 0.0f };
    Vector3 translate = { 0.0f, 0.0f, 6.0f };
    Vector3 cameraPos = { 0.0f, 0.0f, 0.0f };

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();

        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // クロス積出力
        Novice::ScreenPrintf(0, 0, "cross: %0.2f, %0.2f, %0.2f", cross.x, cross.y, cross.z);

        // 自動回転
        rotate.y += 0.02f;

        // キー操作による移動
        if (keys[DIK_W])
            translate.z -= 0.1f;
        if (keys[DIK_S])
            translate.z += 0.1f;
        if (keys[DIK_A])
            translate.x -= 0.1f;
        if (keys[DIK_D])
            translate.x += 0.1f;

        Matrix4x4 worldMatrix = Multiply(MakeRotateMatrix(rotate.y), MakeTranslateMatrix(translate));
        Matrix4x4 cameraMatrix = MakeTranslateMatrix({ -cameraPos.x, -cameraPos.y, -cameraPos.z });
        Matrix4x4 viewMatrix = cameraMatrix;
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.63f, 1280.0f / 720.0f, 0.1f, 100.0f);

        Matrix4x4 viewProjection = Multiply(viewMatrix, projectionMatrix);
        Matrix4x4 worldViewProjection = Multiply(worldMatrix, viewProjection);
        Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280, 720, 0.0f, 1.0f);

        Vector3 screenVertices[3];
        for (int i = 0; i < 3; ++i) {
            Vector3 ndc = Transform(triangle[i], worldViewProjection);
            screenVertices[i] = Transform(ndc, viewportMatrix);
        }

        Novice::DrawTriangle(
            static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
            static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
            static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
            RED, kFillModeSolid);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
            break;
    }

    Novice::Finalize();
    return 0;
}
