import React from 'react';
import { createStackNavigator, createAppContainer, createBottomTabNavigator, createSwitchNavigator } from "react-navigation";
import TabsScreen from './TabsScreen';
import DetailScreen from './DetailScreen';


const AppNavigator = createStackNavigator({
    Tabs: TabsScreen,
    Detail: DetailScreen,
  },
  {
    initialRouteName: "Tabs"
  }
);


const AppContainer = createAppContainer(AppNavigator);

export default class App extends React.Component {
  render() {
    return <AppContainer />;
  }
}

