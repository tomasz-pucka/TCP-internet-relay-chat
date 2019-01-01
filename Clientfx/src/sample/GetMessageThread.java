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
        System.out.println("Get Message Thread");
        while(true) {
            BufferedReader reader = null;
            try {
                reader = new BufferedReader(new InputStreamReader(Controller.clientSocket.getInputStream()));
                String serverMessage = reader.readLine();
                System.out.println(serverMessage);
                while(Controller.wait);
                if(!serverMessage.equals(Controller.msgFromThread)){
                    Controller.msgFromThread=serverMessage;
                    Controller.wait = true;
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}