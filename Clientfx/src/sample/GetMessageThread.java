package sample;

import javafx.scene.control.Label;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class GetMessageThread implements Runnable {



    public GetMessageThread() {

    }

    @Override
    public void run() {
        System.out.println("Watek");
        while(true) {
            BufferedReader reader = null;
            try {
                reader = new BufferedReader(new InputStreamReader(Controller.clientSocket.getInputStream()));
                String serverMessage = reader.readLine();
                System.out.println(serverMessage);
                Controller.msgFromThread=serverMessage;

            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                //usypiamy wątek na 100 milisekund
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}