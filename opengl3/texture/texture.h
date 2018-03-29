#pragma once

enum TextureFiltering {
    TEXTURE_FILTER_MAG_NEAREST = 0, // Nearest criterion for magnification
    TEXTURE_FILTER_MAG_BILINEAR, // Bilinear criterion for magnification
    TEXTURE_FILTER_MIN_NEAREST, // Nearest criterion for minification
    TEXTURE_FILTER_MIN_BILINEAR, // Bilinear criterion for minification
    TEXTURE_FILTER_MIN_NEAREST_MIPMAP, // Nearest criterion for minification, but on closest mipmap
    TEXTURE_FILTER_MIN_BILINEAR_MIPMAP, // Bilinear criterion for minification, but on closest mipmap
    TEXTURE_FILTER_MIN_TRILINEAR, // Bilinear criterion for minification on two closest mipmaps, then averaged
};

// Wraps OpenGL texture object and performs their loading.
class Texture {
public:
    void CreateEmptyTexture(int width, int height, GLenum format);

    void CreateFromData(BYTE *bData, int width, int height, int a_iBPP, GLenum format, bool bGenerateMipMaps = false);

    bool LoadTexture2D(string path, bool bGenerateMipMaps = false);

    void BindTexture(int iTextureUnit = 0);

    void SetFiltering(int a_tfMagnification, int a_tfMinification);

    void SetSamplerParameter(GLenum parameter, GLenum value);

    int GetMinificationFilter();

    int GetMagnificationFilter();

    int GetWidth();

    int GetHeight();

    int GetBPP();

    UINT GetTextureID();

    void DeleteTexture();

    Texture();

private:

    int iWidth, iHeight, iBPP; // Texture width, height, and bytes per pixel
    UINT uiTexture; // Texture name
    UINT uiSampler; // Sampler name
    bool bMipMapsGenerated;

    int tfMinification, tfMagnification;

    string sPath;
};
