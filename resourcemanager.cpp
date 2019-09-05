#include "resourcemanager.h"
#include "innpch.h"
#include <QDebug>

ResourceManager::~ResourceManager()
{
    mShaders.clear();

    for(auto& texture : mTextures)
    {
        delete texture.second;
    }
}

void ResourceManager::addShader(const std::string &name, std::shared_ptr<Shader> shader)
{
    if(shader && mShaders.find(name) == mShaders.end())
    {
        mShaders[name] = shader;

        qDebug() << "ResourceManager: Added shader " << QString::fromStdString(name) << " (id: " << shader->getProgram() << ")";
    }
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string &name)
{
    if(mShaders.find(name) != mShaders.end())
    {
        return mShaders[name];
    }

    return nullptr;
}

void ResourceManager::loadTexture(const std::string &name, const std::string &path)
{
    if(mTextures.find(name) == mTextures.end())
    {
        if(!mIsInitialized)
        {
            initializeOpenGLFunctions();
            mIsInitialized = true;
        }

        mTextures[name] = new Texture(path);

        glActiveTexture(static_cast<GLuint>(GL_TEXTURE0 + mTextures.size() - 1));
        glBindTexture(GL_TEXTURE_2D, mTextures[name]->id());
    }
}

int ResourceManager::getTexture(const std::string &name)
{
    if(mTextures.find(name) != mTextures.end())
    {
        return static_cast<int>(mTextures[name]->id());
    }

    return -1;
}

void ResourceManager::addMesh(const std::string& name, const std::string& path, GLenum renderType)
{
    if(mMeshes.find(name) == mMeshes.end())
    {
        if(!openglInitialized)
        {
            initializeOpenGLFunctions();
            openglInitialized = true;
        }

        std::pair<std::vector<Vertex>, std::vector<GLuint>> data;
        if(QString::fromStdString(path).contains(".obj"))
        {
            data = readObjFile(path);
        }
        else
        {
            data = readTxtFile(path);
        }

        MeshData meshData;

        meshData.mRenderType = renderType;

        //Vertex Array Object - VAO
        glGenVertexArrays( 1, &meshData.mVAO );
        glBindVertexArray(meshData.mVAO);

        //Vertex Buffer Object to hold vertices - VBO
        GLuint vbo;
        glGenBuffers( 1, &vbo );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );

        glBufferData( GL_ARRAY_BUFFER, data.first.size() * sizeof(Vertex), data.first.data(), GL_STATIC_DRAW );

        // 1rst attribute buffer : vertices
        glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // 2nd attribute buffer : colors
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex),  (GLvoid*)(3 * sizeof(GLfloat)) );
        glEnableVertexAttribArray(1);

        // 3rd attribute buffer : uvs
        glVertexAttribPointer(2, 2,  GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)( 6 * sizeof( GLfloat ) ));
        glEnableVertexAttribArray(2);

        meshData.mVerticesCount = data.first.size();
        meshData.mIndicesCount = data.second.size();

        if(meshData.mIndicesCount)
        {
            //Second buffer - holds the indices (Element Array Buffer - EAB):
            GLuint eab;
            glGenBuffers(1, &eab);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.mIndicesCount * sizeof(GLuint), data.second.data(), GL_STATIC_DRAW);
        }


        mMeshes[name] = std::make_shared<MeshData>(meshData);
        glBindVertexArray(0);
    }
}

std::shared_ptr<MeshData> ResourceManager::getMesh(const std::string& name)
{
    if(mMeshes.find(name) != mMeshes.end())
    {
        return mMeshes[name];
    }
    return nullptr;
}

std::pair<std::vector<Vertex>, std::vector<GLuint>> ResourceManager::readObjFile(std::string filename)
{
    std::vector<Vertex> mVertices;
    std::vector<GLuint> mIndices;

    //Open File
    std::string fileWithPath = gsl::assetFilePath + "Meshes/" + filename;
    std::ifstream fileIn;
    fileIn.open (fileWithPath, std::ifstream::in);
    if(!fileIn)
        qDebug() << "Could not open file for reading: " << QString::fromStdString(filename);

    //One line at a time-variable
    std::string oneLine;
    //One word at a time-variable
    std::string oneWord;

    std::vector<gsl::Vector3D> tempVertecies;
    std::vector<gsl::Vector3D> tempNormals;
    std::vector<gsl::Vector2D> tempUVs;

    //    std::vector<Vertex> mVertices;    //made in VisualObject
    //    std::vector<GLushort> mIndices;   //made in VisualObject

    // Varible for constructing the indices vector
    unsigned int temp_index = 0;

    //Reading one line at a time from file to oneLine
    while(std::getline(fileIn, oneLine))
    {
        //Doing a trick to get one word at a time
        std::stringstream sStream;
        //Pushing line into stream
        sStream << oneLine;
        //Streaming one word out of line
        oneWord = ""; //resetting the value or else the last value might survive!
        sStream >> oneWord;

        if (oneWord == "#")
        {
            //Ignore this line
            //            qDebug() << "Line is comment "  << QString::fromStdString(oneWord);
            continue;
        }
        if (oneWord == "")
        {
            //Ignore this line
            //            qDebug() << "Line is blank ";
            continue;
        }
        if (oneWord == "v")
        {
            //            qDebug() << "Line is vertex "  << QString::fromStdString(oneWord) << " ";
            gsl::Vector3D tempVertex;
            sStream >> oneWord;
            tempVertex.x = std::stof(oneWord);
            sStream >> oneWord;
            tempVertex.y = std::stof(oneWord);
            sStream >> oneWord;
            tempVertex.z = std::stof(oneWord);

            //Vertex made - pushing it into vertex-vector
            tempVertecies.push_back(tempVertex);

            continue;
        }
        if (oneWord == "vt")
        {
            //            qDebug() << "Line is UV-coordinate "  << QString::fromStdString(oneWord) << " ";
            gsl::Vector2D tempUV;
            sStream >> oneWord;
            tempUV.x = std::stof(oneWord);
            sStream >> oneWord;
            tempUV.y = std::stof(oneWord);

            //UV made - pushing it into UV-vector
            tempUVs.push_back(tempUV);

            continue;
        }
        if (oneWord == "vn")
        {
            //            qDebug() << "Line is normal "  << QString::fromStdString(oneWord) << " ";
            gsl::Vector3D tempNormal;
            sStream >> oneWord;
            tempNormal.x = std::stof(oneWord);
            sStream >> oneWord;
            tempNormal.y = std::stof(oneWord);
            sStream >> oneWord;
            tempNormal.z = std::stof(oneWord);

            //Vertex made - pushing it into vertex-vector
            tempNormals.push_back(tempNormal);
            continue;
        }
        if (oneWord == "f")
        {
            //            qDebug() << "Line is a face "  << QString::fromStdString(oneWord) << " ";
            //int slash; //used to get the / from the v/t/n - format
            int index, normal, uv;
            for(int i = 0; i < 3; i++)
            {
                sStream >> oneWord;     //one word read
                std::stringstream tempWord(oneWord);    //to use getline on this one word
                std::string segment;    //the numbers in the f-line
                std::vector<std::string> segmentArray;  //temp array of the numbers
                while(std::getline(tempWord, segment, '/')) //splitting word in segments
                {
                    segmentArray.push_back(segment);
                }
                index = std::stoi(segmentArray[0]);     //first is vertex
                if (segmentArray[1] != "")              //second is uv
                    uv = std::stoi(segmentArray[1]);
                else
                {
                    //qDebug() << "No uvs in mesh";       //uv not present
                    uv = 0;                             //this will become -1 in a couple of lines
                }
                normal = std::stoi(segmentArray[2]);    //third is normal

                //Fixing the indexes
                //because obj f-lines starts with 1, not 0
                --index;
                --uv;
                --normal;

                if (uv > -1)    //uv present!
                {
                    Vertex tempVert(tempVertecies[index], tempNormals[normal], tempUVs[uv]);
                    mVertices.push_back(tempVert);
                }
                else            //no uv in mesh data, use 0, 0 as uv
                {
                    Vertex tempVert(tempVertecies[index], tempNormals[normal], gsl::Vector2D(0.0f, 0.0f));
                    mVertices.push_back(tempVert);
                }
                mIndices.push_back(temp_index++);
            }

            //For some reason the winding order is backwards so fixing this by swapping the last two indices
            //Update: this was because the matrix library was wrong - now it is corrected so this is no longer needed.
//            unsigned int back = mIndices.size() - 1;
//            std::swap(mIndices.at(back), mIndices.at(back-1));
            continue;
        }
    }

    //beeing a nice boy and closing the file after use
    fileIn.close();
    qDebug() << "Obj file read: " << QString::fromStdString(filename);

    return {mVertices, mIndices};
}

std::pair<std::vector<Vertex>, std::vector<GLuint>> ResourceManager::readTxtFile(std::string filename)
{
    std::vector<Vertex> mVertices;
    std::vector<GLuint> mIndices;

    std::ifstream inn;
    std::string fileWithPath = gsl::assetFilePath + "Meshes/" + filename;

    inn.open(fileWithPath);

    if (inn.is_open()) {
        int n;
        Vertex vertex;
        inn >> n;
        mVertices.reserve(n);
        for (int i=0; i<n; i++) {
            inn >> vertex;
            mVertices.push_back(vertex);
        }
        inn.close();
        qDebug() << "TriangleSurface file read: " << QString::fromStdString(filename);
    }
    else
    {
        qDebug() << "Could not open file for reading: " << QString::fromStdString(filename);
    }

    return {mVertices, mIndices};
}

ResourceManager::ResourceManager(){}