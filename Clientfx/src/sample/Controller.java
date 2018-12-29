package sample;

import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Node;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;

import javax.swing.*;
import java.awt.*;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Collection;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Controller {


    @FXML
    Button btn_sckt_crt;
    @FXML
    Button btn_room;
    @FXML
    TextField port;
    @FXML
    TextField ip;
    @FXML
    TextField nick;
    @FXML
    TextField room;
    @FXML
    Label room_list;
    @FXML
    Label users;
    @FXML
    Button btn_snd_mssg;
    @FXML
    TextField message;
    @FXML
    TextArea msg;

    public static Socket clientSocket;
    private static String roomList;
    private static boolean changeLabel = false;
    private static String userList;
    private static boolean changeUserList = false;
    private static boolean startChat = false;
    private static String nickStr;
    public static String msgFromThread = "";

    private static String tempMsg = "startValue";
    public void buttonCreateSocket(ActionEvent event) throws IOException {

        System.out.println("Tworze socket");
        System.out.println("ip: " + ip.getText());
        System.out.println("port: " + port.getText());

        clientSocket = new Socket(ip.getText(), Integer.parseInt(port.getText()));

        BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        String serverMessage = reader.readLine();
        if(serverMessage.length()>2) {

            roomList = serverMessage.substring(2, serverMessage.length()-1);
            roomList =  roomList.replaceAll(";", ", ");
            System.out.println("Dostepne pokoje: " +  roomList);
            changeLabel = true;
        }

        Parent tableViewParent = FXMLLoader.load(getClass().getResource("roomChoosing.fxml"));
        Scene tableViewScene = new Scene(tableViewParent);
        Stage window = (Stage) ((Node) event.getSource()).getScene().getWindow();
        window.setScene(tableViewScene);
        window.show();
    }


    public void buttonSendNickname(ActionEvent event) throws IOException {

        changeLabel = false;
        System.out.println("\nWchodze do pokoju");
        System.out.println("nick " + nick.getText());
        System.out.println("room " + room.getText());
        nickStr = nick.getText();

        String clientMessage = room.getText()+";"+nick.getText();
        PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true);
        writer.println(clientMessage);


        BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        String serverMessage = reader.readLine();
        if(serverMessage.length()>2) {

            userList = serverMessage.substring(2);
            userList = userList.replaceAll(";", ", ");
            System.out.println("users: " + userList);
            changeUserList = true;
        }


        startChat = true;
        Parent tableViewParent = FXMLLoader.load(getClass().getResource("Chat.fxml"));
        Scene tableViewScene = new Scene(tableViewParent);
        Stage window = (Stage) ((Node) event.getSource()).getScene().getWindow();
        window.setScene(tableViewScene);
        window.show();
    }

    public void initialize() {

        if(changeLabel)
            room_list.setText("Available rooms: " + roomList);
        if(changeUserList)
            users.setText("Users: \n" + "you \n" + userList);
        if(startChat) {
            System.out.println("start chat");
            Thread getMessageThread;
            Runnable getMessageRun;
            getMessageRun = new GetMessageThread();
            getMessageThread = new Thread(getMessageRun);
            getMessageThread.start();
            update();

            System.out.println("TEST");

            startChat = false;
        }
    }

    private void update() {

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(true) {
                    Platform.runLater(new Runnable() {
                        @Override public void run() {
                            if(!tempMsg.equals(msgFromThread)) {
                                tempMsg = msgFromThread;
                                msg.appendText(msgFromThread + "\n");
                            }
                        }
                    });
                    try {
                        Thread.sleep(50);
                    } catch (InterruptedException ex) {
                        Logger.getLogger(Controller.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            }
        });
        thread.start();
    }

    private void updateMessage(String message) {

        msg.appendText(message + "\n");
    }

    public void refreshMessages(ActionEvent event) {

        updateMessage(msgFromThread);
        msgFromThread = "";
    }



    public void sendMessage(ActionEvent event) throws IOException {

        String clientMessage = nickStr+";"+message.getText();
        PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true);
        writer.println(clientMessage);
        updateMessage(clientMessage);

    }

}
