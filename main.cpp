#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <cstddef>
#include <vector>
#include <cstring>
#include <string>
#include <cmath>

using namespace std;

vector<char> alphabet = {'\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' '};
size_t symb_size= ceil(log2(alphabet.size())); // number bit per 1 sybol

void write(const char text[], unsigned char data[], int w, int h, int c, const char name[]){
    size_t text_size = strlen(text) + 1; // find size of the received text

    int itr{0};
    for(int i(0); i < text_size; i++){ // index letter in text
        for(int k(0); k < alphabet.size(); k++){ // loop through the alphabet to find the required letters
            if(text[i] == alphabet[k]){ // index letter in alphabet
                for(int j(0); j < symb_size; j++){ // writing every bit of the letter
                    byte new_b = byte(k) >> ((symb_size - 1) - j) & (byte)0b01;

                    int index = ((itr / w) * w + (itr % w)) * c;
                    byte pix = byte((int)data[index + (itr % c)]); // pixel color byte
                    if(new_b != (byte(pix) & (byte)0b01)){ // if byte differ, replace with the text byte 
                        data[index + (itr % c)] = static_cast<unsigned char>(to_integer<int>(pix ^ (byte)0b1));
                    }
                    itr++;
                }
            }
        }
    }
    stbi_write_png(name, w, h, c, data, w*c);

}

string read(unsigned char data[], int w, int h, int c){
    string text;
    int itr{0};

    for(int i(0); i < (int)((w * c * h)/symb_size); i++){ // loop the every pixel
        byte letter{0b00}; // write pixel bits here 
        for(int j(0); j < symb_size; j++){ // loop only the symb_size bits
            int index = ((itr / w) * w + (itr % w)) * c;

            byte pix = byte((int)data[index + (itr % c)]); // latter pixel color byte
            letter = (letter << 1) ^ (pix  & (byte)0b01);

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
    int w(0), h(0), c(0);
    string name_pic, text_in_pic, new_name;
    int w_or_r(0);

    cout << "Choose act: \n1)Encrypt\n2)Decrypt\n:> ";
    cin >> w_or_r;

    cout << "\nChoose picture name with extension .png:\n:>";
    cin >> name_pic;

    unsigned char *data = stbi_load((name_pic+ ".png").c_str(), &w, &h, &c, 0);

    if (w_or_r-1){
        string text = read(data, w, h, c);
        cout << "\nThe text that was in the image: " << text << endl;
    }else{
        cout << "\nData that can be written to " << name_pic << ":\n" << float(w * h * c) / 8192 << "kb" << "/" << float(w * h * c) / 8388608 << "mb" << " or " << (w * c * h)/symb_size    << " symbols" <<endl;

        cout << "\nEnter text, which necessary hide:\n:>";
        cin.ignore();
        getline(cin, text_in_pic);

        if(text_in_pic.size() >= (w * c * h)/symb_size){
            cerr << "Text for encrypt not to fit into the picture!";
            return 1;
        }

        cout << "\nEnter name new picture:\n:>";
        cin >> new_name;

        cout << "\nThe text was hidden!";

        write((text_in_pic + "\0").c_str(), data, w, h, c, (new_name + ".png").c_str());
    }

    stbi_image_free(data);
}
