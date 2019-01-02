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
import javafx.stage.Stage;


import javax.swing.*;
import java.io.*;
import java.net.Socket;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
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
    TextArea users;
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

    public static boolean wait = false;

    private static String tempMsg = "startValue";

    //wybieranie serwera
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

    //wybranie nicku i pokoju
    public void buttonSendNickname(ActionEvent event) throws IOException {

        changeLabel = false;
        System.out.println("\nWchodze do pokoju");
        System.out.println("nick " + nick.getText());
        System.out.println("room " + room.getText());
        nickStr = nick.getText();

        if(nickStr.length() < 15 && Integer.parseInt(room.getText()) > 0 && Integer.parseInt(room.getText()) < 31
                && !nickStr.contains(";")) {

            OutputStream os0 = clientSocket.getOutputStream();
            String msgEntry = room.getText()+";"+nick.getText();
            os0.write(msgEntry.getBytes());

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
        else if(nickStr.contains(";")) {
            JOptionPane.showMessageDialog(null, "nickname cannot contain semicolon(;)");
        }
        else if(nickStr.length() >= 15) {
            JOptionPane.showMessageDialog(null, "nickname has to have less or equal 15 characters");
        }
        else if(Integer.parseInt(room.getText()) <= 0 || Integer.parseInt(room.getText()) >= 31 ) {
            JOptionPane.showMessageDialog(null, "room number has to be beetwen 1 and 30");
        }
    }

    //metoda wywolywana przy kazdej inicjalizacji sceny
    public void initialize() {

        boolean flag = true;

        //scena wybierania pokoju
        if(changeLabel)
            room_list.setText("Available rooms: " + roomList);

        //przetworzenie odebranej od serwera listy uzytkownikow
        if(changeUserList){

            userList = userList.replaceAll(", ","\n");
            users.setText("USERS:\n you \n" + userList);
            flag = false;
            changeUserList = false;
        }

        //scena chatu
        if(startChat) {

            if(flag)
                users.setText("USERS:\nyou \n");
            Thread getMessageThread;
            Runnable getMessageRun;
            getMessageRun = new GetMessageThread();
            getMessageThread = new Thread(getMessageRun);
            getMessageThread.start();
            update();
            startChat = false;
        }
    }

    //przetwazanie wiadomosci odebranej z serwera
    private void update() {

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(true) {
                    Platform.runLater(new Runnable() {
                        @Override public void run() {
                            if(!tempMsg.equals(msgFromThread)) {

                                String modifiedMsg = msgFromThread;
                                tempMsg = msgFromThread;
                                if(tempMsg.length() > 2) {

                                    System.out.println(tempMsg.substring(0,2));
                                    if(tempMsg.substring(0,2).equals(";;")) {


                                        modifiedMsg = modifiedMsg.substring(2);
                                        int firstSemicolon = modifiedMsg.indexOf(";");
                                        String actionUser = modifiedMsg.substring(0,firstSemicolon);
                                        updateUsers(actionUser, modifiedMsg.substring(firstSemicolon+1));
                                        System.out.println("USER " + actionUser);
                                        System.out.println("Wiadomosc " + modifiedMsg.substring(firstSemicolon+1)+";");
                                    }
                                    else
                                        updateMessage(msgFromThread);
                                }
                                wait = false;
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

    //wyswietlanie wiadomosci w oknie chatu
    private void updateMessage(String message) {

        //zamiana srednikow oddzielajacych nick od wiadomosci na dwukropek
        message = message.replaceAll(";", ": ");
        msg.appendText(message + "\n");
    }

    //odswiezanie listy urzytkownikow
    private void updateUsers(String user, String action) {

        String tempUsers = users.getText();
        String newUsersList = "";

        List<String> list = new ArrayList<>(Arrays.asList(tempUsers.split("\n")));

        System.out.println(list.indexOf(user));

        //dodawanie do listy uzytkownikow
        if(action.equals("join;")) {

            users.appendText(user + "\n");
            msg.appendText(user + " join\n"); //
        }
        //usuwanie z listy uzytkownikow
        if(action.equals("leave;")) {

            list.remove(user);
            for(int i = 0; i <list.size(); i++) {
                System.out.println(i + ": " + list.get(i));
                newUsersList = newUsersList.concat(list.get(i) + "\n");
            }

            users.setText(newUsersList);
            msg.appendText(user + " leaves\n");
        }

    }




    //wysylanie wiomosci
    public void sendMessage(ActionEvent event) throws IOException {

        String clientMessage = message.getText();
        PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true);
        System.out.println(clientMessage);
        writer.println(clientMessage);
        updateMessage(nickStr + ";" + clientMessage);
        message.setText("");
    }

}
