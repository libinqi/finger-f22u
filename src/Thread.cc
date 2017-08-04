#include "Thread.h"

Baton *Thread::baton = NULL;
uv_loop_t *Thread::loop = NULL;
void (*Thread::doExecute)() = NULL;
bool Thread::isWaiting = false;
bool Thread::isRan = false;
bool Thread::isDied = false;

static std::string GBKToUTF8(const std::string &strGBK)
{
    static std::string strOutUTF8 = "";
    WCHAR *str1;
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char *str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[] str1;
    str1 = NULL;
    delete[] str2;
    str2 = NULL;
    return strOutUTF8;
}

static std::string UNICODE_to_UTF8(const WCHAR *pSrc, int stringLength)
{
    static std::string strOutUTF8 = "";

    char *buffer = new char[stringLength + 1];
    WideCharToMultiByte(CP_UTF8, NULL, pSrc, wcslen(pSrc), buffer, stringLength, NULL, NULL);
    buffer[stringLength] = '\0';

    strOutUTF8 = buffer;

    delete[] buffer;

    const string &delim = " \t";
    string r = strOutUTF8.erase(strOutUTF8.find_last_not_of(delim) + 1);
    return r.erase(0, r.find_first_not_of(delim));
}

static std::string base64_encode(unsigned char const *bytes_to_encode, unsigned int in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    while (in_len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}
static std::string bmp_encode(unsigned char const *img_data_bytes)
{
    // bmp 8位,大小256*288头文件信息
    const std::string file_header = "Qk0AAAAAAAAAADYEAAAoAAAAAAEAACABAAABAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEBAQACAgIAAwMDAAQEBAAFBQUABgYGAAcHBwAICAgACQkJAAoKCgALCwsADAwMAA0NDQAODg4ADw8PABAQEAAREREAEhISABMTEwAUFBQAFRUVABYWFgAXFxcAGBgYABkZGQAaGhoAGxsbABwcHAAdHR0AHh4eAB8fHwAgICAAISEhACIiIgAjIyMAJCQkACUlJQAmJiYAJycnACgoKAApKSkAKioqACsrKwAsLCwALS0tAC4uLgAvLy8AMDAwADExMQAyMjIAMzMzADQ0NAA1NTUANjY2ADc3NwA4ODgAOTk5ADo6OgA7OzsAPDw8AD09PQA+Pj4APz8/AEBAQABBQUEAQkJCAENDQwBEREQARUVFAEZGRgBHR0cASEhIAElJSQBKSkoAS0tLAExMTABNTU0ATk5OAE9PTwBQUFAAUVFRAFJSUgBTU1MAVFRUAFVVVQBWVlYAV1dXAFhYWABZWVkAWlpaAFtbWwBcXFwAXV1dAF5eXgBfX18AYGBgAGFhYQBiYmIAY2NjAGRkZABlZWUAZmZmAGdnZwBoaGgAaWlpAGpqagBra2sAbGxsAG1tbQBubm4Ab29vAHBwcABxcXEAcnJyAHNzcwB0dHQAdXV1AHZ2dgB3d3cAeHh4AHl5eQB6enoAe3t7AHx8fAB9fX0Afn5+AH9/fwCAgIAAgYGBAIKCggCDg4MAhISEAIWFhQCGhoYAh4eHAIiIiACJiYkAioqKAIuLiwCMjIwAjY2NAI6OjgCPj48AkJCQAJGRkQCSkpIAk5OTAJSUlACVlZUAlpaWAJeXlwCYmJgAmZmZAJqamgCbm5sAnJycAJ2dnQCenp4An5+fAKCgoAChoaEAoqKiAKOjowCkpKQApaWlAKampgCnp6cAqKioAKmpqQCqqqoAq6urAKysrACtra0Arq6uAK+vrwCwsLAAsbGxALKysgCzs7MAtLS0ALW1tQC2trYAt7e3ALi4uAC5ubkAurq6ALu7uwC8vLwAvb29AL6+vgC/v78AwMDAAMHBwQDCwsIAw8PDAMTExADFxcUAxsbGAMfHxwDIyMgAycnJAMrKygDLy8sAzMzMAM3NzQDOzs4Az8/PANDQ0ADR0dEA0tLSANPT0wDU1NQA1dXVANbW1gDX19cA2NjYANnZ2QDa2toA29vbANzc3ADd3d0A3t7eAN/f3wDg4OAA4eHhAOLi4gDj4+MA5OTkAOXl5QDm5uYA5+fnAOjo6ADp6ekA6urqAOvr6wDs7OwA7e3tAO7u7gDv7+8A8PDwAPHx8QDy8vIA8/PzAPT09AD19fUA9vb2APf39wD4+PgA+fn5APr6+gD7+/sA/Pz8AP39/QD+/v4A////";
    const std::string file_info = "/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////w==";

    // 图片反转
    unsigned int width = 256;
    unsigned int height = 288;
    unsigned int size = width * height;
    unsigned char imageBytes[256 * 288];

    int srcPos = 0;
    int dstPos = 0;
    for (int i = 0; i <= height - 1; i++)
    {
        srcPos = i * width;
        dstPos = (height - 1 - i) * width;
        memmove(imageBytes + dstPos, srcPos == 0 ? img_data_bytes : img_data_bytes + srcPos, width);
    }

    // 图片base64编码处理
    std::string bmpStr = base64_encode(imageBytes, size);
    bmpStr = file_header + bmpStr;
    bmpStr += file_info;
    return bmpStr;
}
// static unsigned char *bmp_encode(unsigned char const *img_data_bytes)
// {
//     unsigned int width = 256;
//     unsigned int height = 288;

//     unsigned char file_header[] = {
//         //file header
//         0x42, 0x4d, //file type BM (0 - 1)
//         //0x36,0x6c,0x01,0x00, //file size***
//         0x0, 0x0, 0x0, 0x00,    //file size (2 - 5)
//         0x00, 0x00, 0x00, 0x00, //reserved (6 - 9)
//         0x36,                   // 0x0A bytes Header information block size
//         0x4, 0x00, 0x00,        //head byte
//         0x28, 0x00, 0x00, 0x00, //struct size
//         0x00, 0x00, 0x00, 0x00, //map width
//         0x00, 0x00, 0x00, 0x00, //map height
//         0x01, 0x00,             //must be 1
//         0x08, 0x00,             //color count
//         0x00, 0x00, 0x00, 0x00, //compression
//         //0x00,0x68,0x01,0x00, //data size
//         0x00, 0x00, 0x00, 0x00, //data size
//         0x00, 0x00, 0x00, 0x00, //dpix
//         0x00, 0x00, 0x00, 0x00, //dpiy
//         0x00, 0x00, 0x00, 0x00, //color used
//         0x00, 0x00, 0x00, 0x00, //color important
//     };

//     unsigned char imageBytes[74806];
//     memset(imageBytes, 0xFF, 74806);
//     memmove(imageBytes, file_header, 54);

//     unsigned int num = width;
//     imageBytes[18] = (byte)num;
//     num = num >> 8;
//     imageBytes[19] = (byte)num;
//     num = num >> 8;
//     imageBytes[20] = (byte)num;
//     num = num >> 8;
//     imageBytes[21] = (byte)num;

//     num = height;
//     imageBytes[22] = (byte)num;
//     num = num >> 8;
//     imageBytes[23] = (byte)num;
//     num = num >> 8;
//     imageBytes[24] = (byte)num;
//     num = num >> 8;
//     imageBytes[25] = (byte)num;

//     int j = 0;
//     for (int i = 54; i < 1078; i = i + 4)
//     {
//         imageBytes[i] = (byte)j;
//         imageBytes[i + 1] = (byte)j;
//         imageBytes[i + 2] = (byte)j;
//         imageBytes[i + 3] = 0;
//         j++;
//     }

//     int srcPos = 0;
//     int dstPos = 0;
//     for (int i = 0; i <= height - 1; i++)
//     {
//         srcPos = i * width;
//         dstPos = 1078 + (height - 1 - i) * width;
//         memmove(imageBytes + dstPos, srcPos == 0 ? img_data_bytes : img_data_bytes + srcPos, width);
//     }

//     return imageBytes;
// }

Local<Value> Thread::getObject(Isolate *isolate, Receiver receiver)
{
    Local<Object> data = Object::New(isolate);

    switch (baton->receiver.type)
    {
    case ReceiveType::Image:
    {
        std::string mbStr = bmp_encode(receiver.fingerImageData);
        data->Set(String::NewFromUtf8(isolate, "type"), Number::New(isolate, receiver.type));
        data->Set(String::NewFromUtf8(isolate, "isHeightImage"), Number::New(isolate, receiver.isHeightImage));
        data->Set(String::NewFromUtf8(isolate, "data"), String::NewFromUtf8(isolate, mbStr.c_str()));
        break;
    }

    case ReceiveType::Feature:
    {
        std::string mbStr = base64_encode(receiver.fingerTemplateData, sizeof(receiver.fingerTemplateData));
        data->Set(String::NewFromUtf8(isolate, "type"), Number::New(isolate, receiver.type));
        data->Set(String::NewFromUtf8(isolate, "data"), String::NewFromUtf8(isolate, mbStr.c_str()));
        break;
    }

    case ReceiveType::Verify:
    {

        break;
    }
    }

    return {data};
}

void Thread::callback(uv_async_t *request, int status)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Baton *baton = static_cast<Baton *>(request->data);
    Local<Value> argv[2];

    if (!baton->onReceive.IsEmpty() && (baton->state == 0))
    {
        // argv[0] = Number::New(isolate, baton->receiver.type);
        argv[0] = getObject(isolate, baton->receiver);
        // argv[1] = baton->receiver.data.;
        Local<Function>::New(isolate, baton->onReceive)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    }

    if (!baton->onError.IsEmpty() && (baton->state == 1))
    {
        argv[0] = Number::New(isolate, baton->error.code);
        argv[1] = String::NewFromUtf8(isolate, baton->error.message.c_str());
        Local<Function>::New(isolate, baton->onError)->Call(isolate->GetCurrentContext()->Global(), 2, argv);
    }
}

Thread::Thread()
{
}

void Thread::run(uv_work_t *request)
{
    isDied = false;

    while (true)
    {
        try
        {
            if (isDied)
            {
                break;
            }

            uv_mutex_lock(&baton->mutex);
            if (baton->isWaiting)
            {
                uv_cond_wait(&baton->condvar, &baton->mutex);
                baton->isWaiting = false;
            }
            uv_mutex_unlock(&baton->mutex);

            doExecute();
            Sleep(10);
        }
        catch (exception &e)
        {
            cout << "exception: " << e.what() << endl;
        }
    }

    isDied = true;
}

void Thread::stop(uv_work_t *request, int status)
{
    isDied = true;
    Baton *baton = static_cast<Baton *>(request->data);
    uv_close((uv_handle_t *)&baton->asyncRequest, NULL);
}

void Thread::bind(Isolate *isolate, Local<Function> onReceive, Local<Function> onError)
{
    baton = new Baton();
    baton->onReceive.Reset(isolate, Persistent<Function>::Persistent(isolate, onReceive));
    baton->onError.Reset(isolate, Persistent<Function>::Persistent(isolate, onError));
    baton->request.data = baton;

    loop = uv_default_loop();
    uv_async_init(loop, &baton->asyncRequest, (uv_async_cb)callback);
    uv_cond_init(&baton->condvar);
    uv_mutex_init(&baton->mutex);
}

void Thread::resume(void)
{
    if (!isRan)
    {
        uv_queue_work(loop, &baton->request, run, stop);
        uv_run(loop, UV_RUN_NOWAIT);
        isRan = true;
    }

    uv_mutex_lock(&baton->mutex);
    uv_cond_signal(&baton->condvar);
    uv_mutex_unlock(&baton->mutex);
}

void Thread::suspend(void)
{
    uv_mutex_lock(&baton->mutex);
    baton->isWaiting = true;
    uv_mutex_unlock(&baton->mutex);
}

void Thread::doReceive(Receiver receiver)
{
    baton->state = 0;
    baton->receiver = receiver;
    baton->asyncRequest.data = baton;
    uv_async_send(&baton->asyncRequest);
}

void Thread::doError(int code, string message)
{
    baton->state = 1;
    baton->error.code = code;
    baton->error.message = message;
    baton->asyncRequest.data = baton;
    uv_async_send(&baton->asyncRequest);
}

Thread::~Thread()
{
    uv_mutex_destroy(&baton->mutex);
    delete baton;
}