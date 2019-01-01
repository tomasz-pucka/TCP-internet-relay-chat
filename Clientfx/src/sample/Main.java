package sample;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class Main extends Application {

    @Override
    public void start(Stage primaryStage) throws Exception{
        Parent root = FXMLLoader.load(getClass().getResource("serverChoosing.fxml"));
        primaryStage.setTitle("IRC");
        primaryStage.setScene(new Scene(root, 300, 275));
        primaryStage.show();
    }

    @Override
    public void stop() throws IOException {

        Controller.clientSocket.close();
        System.exit(0);
    }

    public static void main(String[] args) {

        System.out.println("Witam Gracza");
        launch(args);
    }
}
