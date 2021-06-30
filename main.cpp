#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <SFML/Audio.hpp>
#include <windows.h>
#include "Binary.hpp"
#include "LBRT.hpp"
#include "BND.hpp"
#include <chrono>
#include <thread>
#include <math.h>

void preciseSleep(double seconds) { ///thanks to https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

using namespace std;

bool debuglog = false;

int main()
{
    LBRT handle;
    BND bnd_handle;

    vector<unsigned char> f_magicA, f_magicB, f_SGD, f_LBRT;

    f_magicA = {0x1A, 0x0F, 0x1B, 0x0F, 0x1D, 0x0F, 0x1F, 0x0F, 0x21, 0x0F, 0x22, 0x0F, 0x24, 0x0F, 0x26, 0x0F, 0x28, 0x0F, 0x29, 0x0F, 0x2B, 0x0F, 0x2D, 0x0F, 0x2F, 0x0F, 0x30, 0x0F, 0x32, 0x0F, 0x34, 0x0F, 0x36, 0x0F, 0x37, 0x0F, 0x39, 0x0F, 0x3B, 0x0F, 0x3D, 0x0F, 0x3E, 0x0F, 0x40, 0x0F, 0x42, 0x0F, 0x44, 0x0F, 0x45, 0x0F, 0x47, 0x0F, 0x49, 0x0F, 0x4B, 0x0F, 0x4D, 0x0F, 0x4E, 0x0F, 0x50, 0x0F, 0x52, 0x0F, 0x54, 0x0F, 0x55, 0x0F, 0x57, 0x0F, 0x59, 0x0F, 0x5B, 0x0F, 0x5C, 0x0F, 0x5E, 0x0F, 0x60, 0x0F, 0x62, 0x0F, 0x64, 0x0F, 0x65, 0x0F, 0x67, 0x0F, 0x69, 0x0F, 0x6B, 0x0F, 0x6C, 0x0F, 0x6E, 0x0F, 0x70, 0x0F, 0x72, 0x0F, 0x74, 0x0F, 0x75, 0x0F, 0x77, 0x0F, 0x79, 0x0F, 0x7B, 0x0F, 0x7D, 0x0F, 0x7E, 0x0F, 0x80, 0x0F, 0x82, 0x0F, 0x84, 0x0F, 0x86, 0x0F, 0x87, 0x0F, 0x89, 0x0F, 0x8B, 0x0F, 0x8D, 0x0F, 0x8E, 0x0F, 0x90, 0x0F, 0x92, 0x0F, 0x94, 0x0F, 0x96, 0x0F, 0x97, 0x0F, 0x99, 0x0F, 0x9B, 0x0F, 0x9D, 0x0F, 0x9F, 0x0F, 0xA1, 0x0F, 0xA2, 0x0F, 0xA4, 0x0F, 0xA6, 0x0F, 0xA8, 0x0F, 0xAA, 0x0F, 0xAB, 0x0F, 0xAD, 0x0F, 0xAF, 0x0F, 0xB1, 0x0F, 0xB3, 0x0F, 0xB4, 0x0F, 0xB6, 0x0F, 0xB8, 0x0F, 0xBA, 0x0F, 0xBC, 0x0F, 0xBD, 0x0F, 0xBF, 0x0F, 0xC1, 0x0F, 0xC3, 0x0F, 0xC5, 0x0F, 0xC7, 0x0F, 0xC8, 0x0F, 0xCA, 0x0F, 0xCC, 0x0F, 0xCE, 0x0F, 0xD0, 0x0F, 0xD2, 0x0F, 0xD3, 0x0F, 0xD5, 0x0F, 0xD7, 0x0F, 0xD9, 0x0F, 0xDB, 0x0F, 0xDD, 0x0F, 0xDE, 0x0F, 0xE0, 0x0F, 0xE2, 0x0F, 0xE4, 0x0F, 0xE6, 0x0F, 0xE8, 0x0F, 0xE9, 0x0F, 0xEB, 0x0F, 0xED, 0x0F, 0xEF, 0x0F, 0xF1, 0x0F, 0xF3, 0x0F, 0xF4, 0x0F, 0xF6, 0x0F, 0xF8, 0x0F, 0xFA, 0x0F, 0xFC, 0x0F, 0xFE, 0x0F, 0x00, 0x10, 0x01, 0x10, 0x03, 0x10, 0x05, 0x10, 0x07, 0x10, 0x09, 0x10, 0x0B, 0x10, 0x0C, 0x10, 0x0E, 0x10, 0x10, 0x10, 0x12, 0x10, 0x14, 0x10, 0x16, 0x10, 0x18, 0x10, 0x19, 0x10, 0x1B, 0x10, 0x1D, 0x10, 0x1F, 0x10, 0x21, 0x10, 0x23, 0x10, 0x25, 0x10, 0x27, 0x10, 0x28, 0x10, 0x2A, 0x10, 0x2C, 0x10, 0x2E, 0x10, 0x30, 0x10, 0x32, 0x10, 0x34, 0x10, 0x35, 0x10, 0x37, 0x10, 0x39, 0x10, 0x3B, 0x10, 0x3D, 0x10, 0x3F, 0x10, 0x41, 0x10, 0x43, 0x10, 0x44, 0x10, 0x46, 0x10, 0x48, 0x10, 0x4A, 0x10, 0x4C, 0x10, 0x4E, 0x10, 0x50, 0x10, 0x52, 0x10, 0x54, 0x10, 0x55, 0x10, 0x57, 0x10, 0x59, 0x10, 0x5B, 0x10, 0x5D, 0x10, 0x5F, 0x10, 0x61, 0x10, 0x63, 0x10, 0x65, 0x10, 0x66, 0x10, 0x68, 0x10, 0x6A, 0x10, 0x6C, 0x10, 0x6E, 0x10, 0x70, 0x10, 0x72, 0x10, 0x74, 0x10, 0x76, 0x10, 0x78, 0x10, 0x79, 0x10, 0x7B, 0x10, 0x7D, 0x10, 0x7F, 0x10, 0x81, 0x10, 0x83, 0x10, 0x85, 0x10, 0x87, 0x10, 0x89, 0x10, 0x8B, 0x10, 0x8D, 0x10, 0x8E, 0x10, 0x90, 0x10, 0x92, 0x10, 0x94, 0x10, 0x96, 0x10, 0x98, 0x10, 0x9A, 0x10, 0x9C, 0x10, 0x9E, 0x10, 0xA0, 0x10, 0xA2, 0x10, 0xA4, 0x10, 0xA5, 0x10, 0xA7, 0x10, 0xA9, 0x10, 0xAB, 0x10, 0xAD, 0x10, 0xAF, 0x10, 0xB1, 0x10, 0xB3, 0x10, 0xB5, 0x10, 0xB7, 0x10, 0xB9, 0x10, 0xBB, 0x10, 0xBD, 0x10, 0xBF, 0x10, 0xC0, 0x10, 0xC2, 0x10, 0xC4, 0x10, 0xC6, 0x10, 0xC8, 0x10, 0xCA, 0x10, 0xCC, 0x10, 0xCE, 0x10, 0xD0, 0x10, 0xD2, 0x10, 0xD4, 0x10, 0xD6, 0x10, 0xD8, 0x10, 0xDA, 0x10, 0xDC, 0x10, 0xDE, 0x10, 0xE0, 0x10, 0xE1, 0x10, 0xE3, 0x10, 0xE5, 0x10, 0xE7, 0x10, 0xE9, 0x10, 0xEB, 0x10, 0xED, 0x10, 0xEF, 0x10, 0xF1, 0x10, 0x00, 0x00, 0x32, 0x00, 0x64, 0x00, 0x96, 0x00, 0xC8, 0x00, 0xFB, 0x00, 0x2D, 0x01, 0x5F, 0x01, 0x91, 0x01, 0xC3, 0x01, 0xF5, 0x01, 0x27, 0x02, 0x59, 0x02, 0x8A, 0x02, 0xBC, 0x02, 0xED, 0x02, 0x1F, 0x03, 0x50, 0x03, 0x81, 0x03, 0xB2, 0x03, 0xE3, 0x03, 0x13, 0x04, 0x44, 0x04, 0x74, 0x04, 0xA5, 0x04, 0xD5, 0x04, 0x04, 0x05, 0x34, 0x05, 0x63, 0x05, 0x93, 0x05, 0xC2, 0x05, 0xF0, 0x05, 0x1F, 0x06, 0x4D, 0x06, 0x7B, 0x06, 0xA9, 0x06, 0xD7, 0x06, 0x04, 0x07, 0x31, 0x07, 0x5E, 0x07, 0x8A, 0x07, 0xB7, 0x07, 0xE2, 0x07, 0x0E, 0x08, 0x39, 0x08, 0x64, 0x08, 0x8F, 0x08, 0xB9, 0x08, 0xE3, 0x08, 0x0D, 0x09, 0x36, 0x09, 0x5F, 0x09, 0x87, 0x09, 0xB0, 0x09, 0xD7, 0x09, 0xFF, 0x09, 0x26, 0x0A, 0x4D, 0x0A, 0x73, 0x0A, 0x99, 0x0A, 0xBE, 0x0A, 0xE3, 0x0A, 0x08, 0x0B, 0x2C, 0x0B, 0x50, 0x0B, 0x73, 0x0B, 0x96, 0x0B, 0xB8, 0x0B, 0xDA, 0x0B, 0xFC, 0x0B, 0x1D, 0x0C, 0x3E, 0x0C, 0x5E, 0x0C, 0x7D, 0x0C, 0x9D, 0x0C, 0xBB, 0x0C, 0xD9, 0x0C, 0xF7, 0x0C, 0x14, 0x0D, 0x31, 0x0D, 0x4D, 0x0D, 0x69, 0x0D, 0x84, 0x0D, 0x9F, 0x0D, 0xB9, 0x0D, 0xD2, 0x0D, 0xEB, 0x0D, 0x04, 0x0E, 0x1C, 0x0E, 0x33, 0x0E, 0x4A, 0x0E, 0x60, 0x0E, 0x76, 0x0E, 0x8B, 0x0E, 0xA0, 0x0E, 0xB4, 0x0E, 0xC8, 0x0E, 0xDB, 0x0E, 0xED, 0x0E, 0xFF, 0x0E, 0x10, 0x0F, 0x21, 0x0F, 0x31, 0x0F, 0x40, 0x0F, 0x4F, 0x0F, 0x5D, 0x0F, 0x6B, 0x0F, 0x78, 0x0F, 0x85, 0x0F, 0x91, 0x0F, 0x9C, 0x0F, 0xA7, 0x0F, 0xB1, 0x0F, 0xBA, 0x0F, 0xC3, 0x0F, 0xCB, 0x0F, 0xD3, 0x0F, 0xDA, 0x0F, 0xE1, 0x0F, 0xE7, 0x0F, 0xEC, 0x0F, 0xF0, 0x0F, 0xF4, 0x0F, 0xF8, 0x0F, 0xFB, 0x0F, 0xFD, 0x0F, 0xFE, 0x0F, 0xFF, 0x0F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x91, 0x01, 0x59, 0x02, 0x1F, 0x03, 0xE3, 0x03, 0xA5, 0x04, 0x63, 0x05, 0x1F, 0x06, 0xD7, 0x06, 0x8A, 0x07, 0x39, 0x08, 0xE3, 0x08, 0x87, 0x09, 0x26, 0x0A, 0xBE, 0x0A, 0x50, 0x0B, 0xDA, 0x0B, 0x5E, 0x0C, 0xD9, 0x0C, 0x4D, 0x0D, 0xB9, 0x0D, 0x1C, 0x0E, 0x76, 0x0E, 0xC8, 0x0E, 0x10, 0x0F, 0x4F, 0x0F, 0x85, 0x0F, 0xB1, 0x0F, 0xD3, 0x0F, 0xEC, 0x0F, 0xFB, 0x0F, 0x00, 0x10, 0xFB, 0x0F, 0xEC, 0x0F, 0xD3, 0x0F, 0xB1, 0x0F, 0x85, 0x0F, 0x4F, 0x0F, 0x10, 0x0F, 0xC8, 0x0E, 0x76, 0x0E, 0x1C, 0x0E, 0xB9, 0x0D, 0x4D, 0x0D, 0xD9, 0x0C, 0x5E, 0x0C, 0xDA, 0x0B, 0x50, 0x0B, 0xBE, 0x0A, 0x26, 0x0A, 0x87, 0x09, 0xE3, 0x08, 0x39, 0x08, 0x8A, 0x07, 0xD7, 0x06, 0x1F, 0x06, 0x63, 0x05, 0xA5, 0x04, 0xE3, 0x03, 0x1F, 0x03, 0x59, 0x02, 0x91, 0x01, 0xC8, 0x00, 0x00, 0x00};
    //Binary::file_to_uchar(Binary::get_file("magic1.dat"));
    f_magicB = {0x00, 0x01, 0x00, 0x00, 0x0F, 0x01, 0x00, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00, 0x42, 0x01, 0x00, 0x00, 0x55, 0x01, 0x00, 0x00, 0x6A, 0x01, 0x00, 0x00, 0x7F, 0x01, 0x00, 0x00, 0x96, 0x01, 0x00, 0x00, 0xAE, 0x01, 0x00, 0x00, 0xC8, 0x01, 0x00, 0x00, 0xE3, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x1E, 0x02, 0x00, 0x00, 0x3E, 0x02, 0x00, 0x00, 0x60, 0x02, 0x00, 0x00, 0x85, 0x02, 0x00, 0x00, 0xAB, 0x02, 0x00, 0x00, 0xD4, 0x02, 0x00, 0x00, 0xFF, 0x02, 0x00, 0x00, 0x2C, 0x03, 0x00, 0x00, 0x5D, 0x03, 0x00, 0x00, 0x90, 0x03, 0x00, 0x00, 0xC6, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3C, 0x04, 0x00, 0x00, 0x7D, 0x04, 0x00, 0x00, 0xC1, 0x04, 0x00, 0x00, 0x0A, 0x05, 0x00, 0x00, 0x56, 0x05, 0x00, 0x00, 0xA8, 0x05, 0x00, 0x00, 0xFE, 0x05, 0x00, 0x00, 0x59, 0x06, 0x00, 0x00, 0xBA, 0x06, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x8D, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x79, 0x08, 0x00, 0x00, 0xFA, 0x08, 0x00, 0x00, 0x83, 0x09, 0x00, 0x00, 0x14, 0x0A, 0x00, 0x00, 0xAD, 0x0A, 0x00, 0x00, 0x50, 0x0B, 0x00, 0x00, 0xFC, 0x0B, 0x00, 0x00, 0xB2, 0x0C, 0x00, 0x00, 0x74, 0x0D, 0x00, 0x00, 0x41, 0x0E, 0x00, 0x00, 0x1A, 0x0F, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0xF3, 0x10, 0x00, 0x00, 0xF5, 0x11, 0x00, 0x00, 0x06, 0x13, 0x00, 0x00, 0x28, 0x14, 0x00, 0x00, 0x5B, 0x15, 0x00, 0x00, 0xA0, 0x16, 0x00, 0x00, 0xF9, 0x17, 0x00, 0x00, 0x65, 0x19, 0x00, 0x00, 0xE8, 0x1A, 0x00, 0x00, 0x82, 0x1C, 0x00, 0x00, 0x34, 0x1E, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0xE7, 0x21, 0x00, 0x00, 0xEB, 0x23, 0x00, 0x00, 0x0D, 0x26, 0x00, 0x00, 0x51, 0x28, 0x00, 0x00, 0xB7, 0x2A, 0x00, 0x00, 0x41, 0x2D, 0x00, 0x00, 0xF2, 0x2F, 0x00, 0x00, 0xCB, 0x32, 0x00, 0x00, 0xD1, 0x35, 0x00, 0x00, 0x04, 0x39, 0x00, 0x00, 0x68, 0x3C, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0xCE, 0x43, 0x00, 0x00, 0xD6, 0x47, 0x00, 0x00, 0x1B, 0x4C, 0x00, 0x00, 0xA2, 0x50, 0x00, 0x00, 0x6E, 0x55, 0x00, 0x00, 0x82, 0x5A, 0x00, 0x00, 0xE4, 0x5F, 0x00, 0x00, 0x97, 0x65, 0x00, 0x00, 0xA2, 0x6B, 0x00, 0x00, 0x08, 0x72, 0x00, 0x00, 0xD0, 0x78, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x9C, 0x87, 0x00, 0x00, 0xAC, 0x8F, 0x00, 0x00, 0x37, 0x98, 0x00, 0x00, 0x45, 0xA1, 0x00, 0x00, 0xDC, 0xAA, 0x00, 0x00, 0x04, 0xB5, 0x00, 0x00, 0xC8, 0xBF, 0x00, 0x00, 0x2F, 0xCB, 0x00, 0x00, 0x44, 0xD7, 0x00, 0x00, 0x11, 0xE4, 0x00, 0x00, 0xA1, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    //Binary::file_to_uchar(Binary::get_file("magic2.dat"));

    cout << "=====================================================" << endl;
    cout << "        LBRTPlayer PROTOTYPE v1.02 by Owocek" << endl;
    cout << "=====================================================" << endl << endl;

    cout << "Verifying data: f_magicA " << f_magicA.size() << " f_magicB " << f_magicB.size() << endl;

    struct LBRTSample
    {
        float pitch;
        float lvol,rvol;
        int sample;
        int timeout;
        bool playable = false;
    };

    vector<LBRTSample> lbrtSong;

    vector<sf::SoundBuffer> vag_samples;

    bnd_handle.load("bgm.dat");
    bnd_handle.extract_all();

    system("SGXDConvert.exe @bgm.dat\\kaesi_1_1.sgd");


    string sample_path = "";
    //cin >> sample_path;

    string sample_am = "";
    //cin >> sample_am;

    string buf;

    ifstream pp("@bgm.dat\\@kaesi_1_1\\name.txt");
    while(getline(pp, buf))
    {
        sample_path = "@bgm.dat\\@kaesi_1_1\\"+buf+"_";
    }
    pp.close();

    ifstream pp2("@bgm.dat\\@kaesi_1_1\\size.txt");
    while(getline(pp2, buf))
    {
        sample_am = buf;
    }
    pp2.close();

    string sgd_path = "@bgm.dat\\kaesi_1_1.sgd";
    //cin >> sgd_path;

    f_SGD = Binary::file_to_uchar(Binary::get_file(sgd_path));

    ///Load up sounds
    for(int i=0; i<atoi(sample_am.c_str()); i++)
    {
        cout << "Vag sample " << sample_path+to_string(i)+".wav" << " loading...";

        sf::SoundBuffer tmp;
        tmp.loadFromFile(sample_path+to_string(i)+".wav");

        vag_samples.push_back(tmp);

        cout << " loaded" << endl;
    }

    vector<string> lbrt_paths = {"@bgm.dat\\bgm_1.lrt", "@bgm.dat\\kaesi_01_1.lrt", "@bgm.dat\\kaesi_01_2.lrt", "@bgm.dat\\kaesi_01_3.lrt"};

    for(int ss=0; ss<lbrt_paths.size(); ss++)
    {
        string lbrt_path = lbrt_paths[ss];
        f_LBRT = Binary::file_to_uchar(Binary::get_file(lbrt_path));

        cout << "Reading file " << lbrt_path << endl;

        ifstream lbrt_file(lbrt_path, ios::binary);

        uint32_t start_offset,entries,speed;

        lbrt_file.seekg(0x4);
        lbrt_file.read(reinterpret_cast<char*>(&start_offset), sizeof(uint32_t));

        lbrt_file.seekg(0xC);
        lbrt_file.read(reinterpret_cast<char*>(&speed), sizeof(uint32_t));

        float tempo = 500.f/float(speed);

        lbrt_file.seekg(0x1C);
        lbrt_file.read(reinterpret_cast<char*>(&entries), sizeof(uint32_t));

        cout << "Start offset: 0x" << hex << start_offset << " entries: " << entries << endl;

        cout << "Processing LBRT..." << endl;

        for(uint32_t i=0; i<entries; i++)
        {
            uint32_t entry_offset = start_offset + (i*0x1C);

            uint32_t segment;

            lbrt_file.seekg(entry_offset+0x0);
            lbrt_file.read(reinterpret_cast<char*>(&segment), sizeof(uint32_t));

            uint32_t timeout;

            lbrt_file.seekg(entry_offset+0x4);
            lbrt_file.read(reinterpret_cast<char*>(&timeout), sizeof(uint32_t));

            if(timeout > 0)
            {
                if(debuglog)
                cout << dec << "[Segment " << segment << "] Wait " << timeout << " ms" << endl;
            }

            float realtimeout = float(timeout);

            //if(realtimeout > 0)
            //Sleep(realtimeout);

            if(debuglog)
            cout << "Checking entry " << dec << i << " offset: 0x" << hex << entry_offset << endl;

            LBRTSample tmp;
            tmp.timeout = realtimeout;

            uint8_t check;

            lbrt_file.seekg(entry_offset+0x19);
            lbrt_file.read(reinterpret_cast<char*>(&check), sizeof(uint8_t));

            if((check != 0xFF) && (check != 0xB0) && (check != 0x00))
            {
                ///a playable sample
                uint16_t sampleID, sample_value;

                lbrt_file.seekg(entry_offset+0x12);
                lbrt_file.read(reinterpret_cast<char*>(&sampleID), sizeof(uint16_t));

                lbrt_file.seekg(entry_offset+0x0C);
                lbrt_file.read(reinterpret_cast<char*>(&sample_value), sizeof(uint16_t));

                uint16_t lbrt_0x14;

                lbrt_file.seekg(entry_offset+0x14);
                lbrt_file.read(reinterpret_cast<char*>(&lbrt_0x14), sizeof(uint16_t));

                uint8_t lbrt_0x1A;

                lbrt_file.seekg(entry_offset+0x1A);
                lbrt_file.read(reinterpret_cast<char*>(&lbrt_0x1A), sizeof(uint8_t));

                //cout << "Playable sample found, sample ID: 0x" << sampleID << " sample_value: 0x" << sample_value << endl;

                vector<uint32_t> data = handle.findRealSample(sampleID, sample_value, f_SGD);

                uint32_t real_sample = data[0];
                uint32_t rgnd_valueA = data[3];
                uint32_t rgnd_valueB = data[4];

                uint32_t pitch = handle.findPitch(rgnd_valueA, rgnd_valueB, sample_value, f_magicA, f_magicB);
                vector<uint32_t> vol = handle.getLeftPan(sampleID, sample_value, f_SGD, lbrt_0x14, lbrt_0x1A);

                //cout << "== Sample " << real_sample << " ==" << endl;
                //cout << "Left volume: " << dec << vol[0] << dec << endl;
                //cout << "Right volume: " << dec << vol[1] << dec << endl;

                int intpitch = pitch;
                float factor = float(intpitch) / 4096.f;

                float lpos = (float(vol[0]) / 4096.f) * (100.f);
                float rpos = (float(vol[1]) / 4096.f) * (100.f);

                tmp.lvol = lpos;
                tmp.rvol = rpos;
                tmp.sample = real_sample;
                tmp.pitch = factor;
                tmp.playable = true;

                /**/

                //cout << dec << "[Segment " << segment << "] Play sample " << real_sample << endl;
            }

            lbrtSong.push_back(tmp);
        }

        lbrt_file.close();
    }

    cout << "Begin LBRT playback." << endl;

    int id = 0;
    map<int, sf::Sound> snd_left;
    map<int, sf::Sound> snd_right;

    for(int i=0; i<lbrtSong.size(); i++)
    {
        if(lbrtSong[i].timeout > 0)
        {
            //cout << dec << "[LBRT] " << lbrtSong[i].timeout << "ms sleep" << endl;
            preciseSleep(lbrtSong[i].timeout / 1000.f);
        }

        if(lbrtSong[i].playable)
        {
            //cout << "[LBRT] Sample " << lbrtSong[i].sample << ", pitch: " << lbrtSong[i].pitch << ", leftVol: " << lbrtSong[i].lvol << ", rightVol: " << lbrtSong[i].rvol << endl;

            snd_left[id].stop();
            snd_left[id].setBuffer(vag_samples[lbrtSong[i].sample]);
            snd_left[id].setVolume(lbrtSong[i].lvol);
            snd_left[id].setPosition(-1.f, 0.f, 1.f);
            snd_left[id].setPitch(lbrtSong[i].pitch);
            snd_left[id].play();

            snd_right[id].stop();
            snd_right[id].setBuffer(vag_samples[lbrtSong[i].sample]);
            snd_right[id].setVolume(lbrtSong[i].rvol);
            snd_right[id].setPosition(1.f, 0.f, 1.f);
            snd_right[id].setPitch(lbrtSong[i].pitch);
            snd_right[id].play();

            id++;
            if(id>=126)
            id = 0;
        }
    }

    return 0;
}