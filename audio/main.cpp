#define DR_WAV_IMPLEMENTATION
#include <iostream>
#include "dr_wav.h"
#include <cstddef>
#include <vector>
#include <cstring>
#include <string>
#include <bitset>
#include <cmath>

using namespace std;

vector<char> alphabet = {'\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' '};
size_t symb_size= ceil(log2(alphabet.size()));

int write(const char text[], drwav_int32* psData, unsigned int c, unsigned int sr, unsigned int fc, const char name[]){
    size_t text_size = strlen(text) + 1;

    int itr{0};
    for(int i(0); i < text_size; i++){
        for(int k(0); k < alphabet.size(); k++){
            if(text[i] == alphabet[k]){
                for(int j(0); j < symb_size; j++){
                    byte new_b = byte(k) >> ((symb_size - 1) - j) & (byte)0b01;

                    byte samp = byte(psData[itr]);
                    if(new_b != (byte(samp) & (byte)0b01)){
                        psData[itr] = to_integer<int>(samp ^ (byte)0b1);
                    }
                    itr++;
                }
            }
        }
    }
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = c;
    format.sampleRate = sr;
    format.bitsPerSample = 32;

    drwav wav;

    if (!drwav_init_file_write(&wav, name, &format, NULL)){
        cerr << "Не удалось открыть файл";
        drwav_free(psData, NULL);
        return 1;
    }

    drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, fc, psData);
    drwav_uninit(&wav);
    return 0;
}

string read(drwav_int32* psData, drwav_uint64 size){
    string text;
    drwav_int32 itr(0);
    for(drwav_uint64 i(0); i < size; i++){
        byte letter{0b00};
        for(int j(0); j < symb_size; j++){
            byte samp = byte(psData[itr]);
            letter = (letter << 1) ^ (samp  & (byte)0b01);
            itr++;
        }
        if (to_integer<int>(letter) == 0){
            return text;
        }
        text += alphabet[to_integer<int>(letter)];
    }
    return "0";
}

int main(){
    unsigned int c(0);
    unsigned int sampleRate(0);
    drwav_uint64 totalPCMFrameCount(0);
    string name_wav, text_in_wav, new_name_wav;
    int w_or_r(0);

    cout << "Choose act: \n1)Encrypt\n2)Decrypt\n:> ";
    cin >> w_or_r;

    cout << "\nEnter audio name:\n:>";
    cin >> name_wav;

    drwav_int32* pSampleData = drwav_open_file_and_read_pcm_frames_s32((name_wav + ".wav").c_str(), &c, &sampleRate, &totalPCMFrameCount, NULL);

    if (pSampleData == NULL) {
        cerr << "Не удалось загрузить файл";
    }

    drwav_uint64 totalSample = totalPCMFrameCount * c;

    if (w_or_r-1){
        string text = read(pSampleData, totalSample);
        cout << "\nThe text that was in the audio: " << text << endl;
        drwav_free(pSampleData, NULL);
        return 0;
    }else{
        cout << "\nData that can be written to " << name_wav << ":\n" << totalSample / 8192 << "kb" << ", " << totalSample / 8388608 << "mb" << " or " << totalSample/symb_size << " symbols" <<endl;

        cout << "\nEnter text, which necessary hide:\n:>";
        cin.ignore();
        getline(cin, text_in_wav);

        if(text_in_wav.size() >= totalSample/symb_size){
            cerr << "Text for encrypt not to fit into the picture!";
            return 2;
        }

        cout << "\nEnter new name audio:\n:>";
        cin >> new_name_wav;

        cout << "\nThe text was hidden!";

        write((text_in_wav + "\0").c_str(), pSampleData, c, sampleRate, totalPCMFrameCount, (new_name_wav + ".wav").c_str());
        drwav_free(pSampleData, NULL);
    }
}
