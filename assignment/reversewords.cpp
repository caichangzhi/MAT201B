// MAT201B assignment/0
// created by Changzhi Cai at Jan 7th 2020

#include <iostream>
#include <string>

using namespace std;

// reverse the whole string
void Reverse(string &str, int start, int end)
{
    while (start < end)
    {
        swap(str[start], str[end]);
        start++;
        end--;
    }
}

int main() {
    while (true) {
        printf("Type a sentence (then hit return): ");
        string line;
        getline(cin, line);
        if (!cin.good()) {
            printf("Done\n");
            return 0;
        }

        // index is the position of the first letter of each word
        int len = line.length();
        int index = 0;

        for(int i = 0; i < len; i++) {
            // if find a space
            if (line[i] == ' ') {
                // reverse the word which before the space
                Reverse(line, index, i - 1);
                // record initial position of the next word
                index = i + 1;
            }
        }

        // reverse the last word
        Reverse(line, index, len - 1);
        cout<<line<<endl;
    }
}