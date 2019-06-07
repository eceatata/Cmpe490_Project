import React from 'react';
import {
    StyleSheet,
    Text,
    View,
    Image,
  } from 'react-native';

export default class DetailScreen extends React.Component {
    static navigationOptions = {
        title: "Room 1",
    };
    state = { 
        itemId: "0",
        temperature: 0,
        humidity: 0,
        count: 0,
        quality: 0,
        created_at: ''
    }
    componentWillMount(){
        const { navigation } = this.props;
        const itemId = navigation.getParam('itemId', 'NO-ID');
        this.setState({itemId: itemId})
        fetch(`https://rakyildizi.eu-gb.mybluemix.net/getRoomData`, {
          headers: { 'content-type': 'application/json' }
        }).then(response => {
            var obj = JSON.parse(response._bodyText)
            this.setState({
              temperature: obj.Temperature,
              humidity: obj.Humidity,
              count: obj.Count,
              quality: ((obj.Score *100) / 255).toFixed(2),
              created_at: new Date(obj["Date"])
            })
          
        }).catch(err => {
          console.error(err);
        });

    }
    render() {
        return (
          <View style={styles.container}>
              <View style={styles.header}>
                <View style={styles.headerContent}>
                    <Image style={styles.avatar}
                      source={require("./kutup1.jpg")}/>
    
                    <Text style={styles.name}>
                      Room 1
                    </Text>
                    <Text style={styles.name}>
                      Quality Level: {`${this.state.quality}`}%
                    </Text>
                    <Text style={styles.name2}>
                      Last Updated: {`${this.state.created_at}`}
                    </Text>
                </View>
              </View>
    
              <View style={styles.body}>
                <View style={styles.bodyContent}>
                  <Text style={styles.textInfo}>
                    Occupancy: {`${this.state.count}`}/35 people
                  </Text>
              
                  <Text style={styles.textInfo}>
                    Temperature: {`${this.state.temperature}`} °C
                  </Text>
                
                  <Text style={styles.textInfo}>
                    Humidity: {`${this.state.humidity}`}%
                  </Text>
                </View>
            </View>
          </View>
        );
      }
}

const styles = StyleSheet.create({
    header:{
      backgroundColor: "#1E90FF",
    },
    headerContent:{
      padding:30,
      alignItems: 'center',
    },
    avatar: {
      width: 130,
      height: 130,
      borderRadius: 63,
      borderWidth: 4,
      borderColor: "white",
      marginBottom:10,
    },
    name:{
      fontSize:22,
      color:"#FFFFFF",
      fontWeight:'600',
    },
    name2:{
      fontSize:12,
      color:"#FFFFFF",
      fontWeight:'600',
    },
    bodyContent: {
      flex: 1,
      alignItems: 'center',
      padding:30,
    },
    textInfo:{
      fontSize:18,
      marginTop:20,
      color: "#696969",
    }
  });