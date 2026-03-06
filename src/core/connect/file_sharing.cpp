#include "file_sharing.h"
#include "core/display.h"
#include <SD.h>
FileSharing::FileSharing() {}

void FileSharing::sendFile() {
    drawMainBorderWithTitle("إرسال ملف");

    if (!beginSend()) return;

    File file = selectFile();
    if (!file) {
        displayError("خطأ في اختيار الملف");
        delay(1000);
        return;
    }

    Message message = createFileMessage(file);

    esp_err_t response;
    sendStatus = STARTED;

    drawMainBorderWithTitle("إرسال ملف");
    padprintln("");
    padprintln("جارٍ الإرسال...");

    delay(100);

    while (file.available()) {
        if (check(EscPress)) sendStatus = ABORTED;

        if (sendStatus == ABORTED || sendStatus == FAILED) {
            message.done = true;
            message.dataSize = 0;
            esp_now_send(dstAddress, (uint8_t *)&message, sizeof(message));
            displayError("خطأ في إرسال الملف");
            break;
        }

        size_t bytesRead = file.readBytes(message.data, ESP_DATA_SIZE);
        message.dataSize = bytesRead;
        message.bytesSent = min(message.bytesSent + bytesRead, message.totalBytes);
        message.done = message.bytesSent == message.totalBytes;

        response = esp_now_send(dstAddress, (uint8_t *)&message, sizeof(message));
        if (response != ESP_OK) {
            Serial.printf("Send file response: %s\n", esp_err_to_name(response));
            sendStatus = FAILED;
        }

        progressHandler(file.position(), file.size(), "جارٍ الإرسال...");
        delay(100);
    }

    if (message.bytesSent == message.totalBytes) displaySuccess("تم إرسال الملف");

    file.close();
    delay(1000);
}

void FileSharing::receiveFile() {
    drawMainBorderWithTitle("استقبال ملف");
    padprintln("");
    padprintln("جارٍ الانتظار...");

    recvFileName = "";
    recvQueue = {};
    recvStatus = CONNECTING;

    if (!beginEspnow()) return;

    delay(100);

    while (1) {
        if (check(EscPress)) recvStatus = ABORTED;

        if (recvStatus == ABORTED || recvStatus == FAILED) {
            displayError("خطأ في استقبال الملف");
            break;
        }
        if (recvStatus == SUCCESS) {
            displaySuccess("تم استقبال الملف");
            break;
        }

        if (!recvQueue.empty()) {
            Message recvFileMessage = recvQueue.front();
            recvQueue.erase(recvQueue.begin());

            progressHandler(recvFileMessage.bytesSent, recvFileMessage.totalBytes, "جارٍ الاستقبال...");

            if (!appendToFile(recvFileMessage)) {
                recvStatus = FAILED;
                Serial.println("Failed appending to file");
            }
            if (recvFileMessage.done) {
                Serial.println("Recv done");
                recvStatus = recvFileMessage.bytesSent == recvFileMessage.totalBytes ? SUCCESS : FAILED;
            }
        }

        delay(100);
    }

    delay(1000);

    if (recvStatus == SUCCESS) {
        drawMainBorderWithTitle("استقبال ملف");
        padprintln("");
        padprintln("تم استقبال الملف: ");
        padprintln(recvFileName);
        padprintln("\n");
        padprintln("اضغط أي مفتاح للخروج");
        while (!check(AnyKeyPress)) vTaskDelay(50 / portTICK_PERIOD_MS);
        ;
    }
}

File FileSharing::selectFile() {
    String filename;
    FS *fs = &LittleFS;
    setupSdCard();
    if (sdcardMounted) {
        options = {
            {"بطاقة SD", [&]() { fs = &SD; }      },
            {"LittleFS", [&]() { fs = &LittleFS; }},
        };
        loopOptions(options);
    }
    filename = loopSD(*fs, true);

    File file = fs->open(filename, FILE_READ);
    return file;
}

bool FileSharing::appendToFile(FileSharing::Message fileMessage) {
    FS *fs;
    if (!getFsStorage(fs)) return false;

    if (recvFileName == "") createFilename(fs, fileMessage);

    File file = (*fs).open(recvFileName, FILE_APPEND);
    if (!file) return false;

    file.write((const uint8_t *)fileMessage.data, fileMessage.dataSize);
    file.close();

    return true;
}

void FileSharing::createFilename(FS *fs, FileSharing::Message fileMessage) {
    String messageFilename = String(fileMessage.filename);
    String messageFilepath = String(fileMessage.filepath);

    String filename = messageFilename.substring(0, messageFilename.lastIndexOf("."));
    String ext = messageFilename.substring(messageFilename.lastIndexOf("."));

    Serial.println("Creating filename");
    Serial.print("Path: ");
    Serial.println(messageFilepath);
    Serial.print("Name: ");
    Serial.println(filename);
    Serial.print("Ext: ");
    Serial.println(ext);

    if (!(*fs).exists(messageFilepath)) (*fs).mkdir(messageFilepath);
    if ((*fs).exists(messageFilepath + "/" + filename + ext)) {
        int i = 1;
        filename += "_";
        while ((*fs).exists(messageFilepath + "/" + filename + String(i) + ext)) i++;
        filename += String(i);
    }

    recvFileName = messageFilepath + "/" + filename + ext;
}
