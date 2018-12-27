package sample;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;

import java.io.IOException;
import java.net.Socket;

public class Controller {

    @FXML
    Button btn_sckt_crt;
    @FXML
    TextField port;
    @FXML
    TextField ip;

    public void buttonCreateSocket(ActionEvent event) {

        System.out.println("ip: " + ip.getText());
        System.out.println("port: " + port.getText());
        /*
        try {
            Socket clientSocket = new Socket(ip.getText(), Integer.parseInt(port.getText()));
        } catch (IOException e) {
            e.printStackTrace();
        }*/
    }

}
