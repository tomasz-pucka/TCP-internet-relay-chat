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
import javafx.scene.control.TextField;
import javafx.stage.Stage;

import javax.xml.bind.SchemaOutputResolver;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.List;

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

    private static Socket clientSocket;
    private static String roomList;
    private static boolean changeLabel = false;
    private static String userList;
    private static boolean changeUserList = false;
    private static boolean startChat = false;
    private static String nickStr;

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


        Parent tableViewParent = FXMLLoader.load(getClass().getResource("Chat.fxml"));
        Scene tableViewScene = new Scene(tableViewParent);
        Stage window = (Stage) ((Node) event.getSource()).getScene().getWindow();
        window.setScene(tableViewScene);
        window.show();
        startChat = true;
    }

    public void initialize() throws IOException {

        if(changeLabel)
            room_list.setText("Available rooms: " + roomList);
        if(changeUserList)
            users.setText("Users: \n" + "you \n" + userList);
        if(startChat) {
            receiveMessage();
        }
    }
    public void receiveMessage() throws IOException {

        while(true) {
            BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            String serverMessage = reader.readLine();
            System.out.println(serverMessage);
        }

    }

    public void sendMessage(ActionEvent event) throws IOException {

        String clientMessage = nickStr+";"+message.getText();
        PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true);
        writer.println(clientMessage);
        receiveMessage();
    }
}
